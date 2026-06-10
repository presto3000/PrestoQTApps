#include "tradetickmodel.h"
#include "watchlistmodel.h"

TradeTickModel::TradeTickModel(WatchlistModel *watchlist, QObject *parent)
    : QAbstractListModel(parent),
    m_watchlist(watchlist)
{}

void TradeTickModel::setSymbol(const QString &s)
{
    if (m_symbol == s.toUpper()) return;
    m_symbol = s.toUpper();

    // Clear tape when switching symbol
    beginResetModel();
    m_ticks.clear();
    endResetModel();

    // Seed bid/ask immediately from watchlist so the spread header
    // shows something useful before the next quote arrives via WS.
    // Without this it stays 0/0 until the next quote fires.
    m_bid = m_ask = 0.0;
    m_bidSize = m_askSize = 0;
    seedSpreadFromWatchlist();

    emit symbolChanged();
    emit countChanged();
    emit spreadChanged();
}

int TradeTickModel::rowCount(const QModelIndex &) const
{
    return m_ticks.size();
}

QVariant TradeTickModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_ticks.size())
        return {};

    const TradeTick &t = m_ticks[index.row()];

    switch (role) {
    case TimeRole:   return t.time.toString("hh:mm:ss");
    case PriceRole:  return t.price;
    case SizeRole:   return t.size;
    case SideRole:   return t.side;
    case SymbolRole: return t.symbol;
    case ExchangeRole: return t.exchange;
    }
    return {};
}

QHash<int, QByteArray> TradeTickModel::roleNames() const
{
    return {
            { TimeRole,   "tickTime"  },
            { PriceRole,  "tickPrice" },
            { SizeRole,   "tickSize"  },
            { SideRole,   "tickSide"  },
            { SymbolRole, "tickSymbol"},
            { ExchangeRole, "tickExchange" },
            };
}

void TradeTickModel::addTick(const TradeTick &tick)
{
    // Only record ticks for the currently selected symbol
    if (tick.symbol.toUpper() != m_symbol)
        return;

    beginInsertRows({}, 0, 0);
    m_ticks.prepend(tick);   // newest first
    endInsertRows();

    if (m_ticks.size() > MAX_TICKS) {
        beginRemoveRows({}, m_ticks.size() - 1, m_ticks.size() - 1);
        m_ticks.removeLast();
        endRemoveRows();
    }

    emit countChanged();
}

void TradeTickModel::seedSpreadFromWatchlist()
{
    if (!m_watchlist || m_symbol.isEmpty())
        return;

    const double bid = m_watchlist->bid(m_symbol);
    const double ask = m_watchlist->ask(m_symbol);

    if (bid > 0.0 || ask > 0.0) {
        m_bid     = bid;
        m_ask     = ask;
        m_bidSize = m_watchlist->bidSize(m_symbol);
        m_askSize = m_watchlist->askSize(m_symbol);
    }
}

void TradeTickModel::updateSpread(const QString &symbol,
                                  double bid, int bidSize,
                                  double ask, int askSize)
{
    if (symbol.toUpper() != m_symbol)
        return;

    m_bid     = bid;
    m_ask     = ask;
    m_bidSize = bidSize;
    m_askSize = askSize;

    emit spreadChanged();
}
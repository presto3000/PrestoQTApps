#include "watchlistmodel.h"
#include <QDebug>
#include <QSettings>

WatchlistModel::WatchlistModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadWatchlist();
}

int WatchlistModel::rowCount(const QModelIndex &) const
{
    return m_entries.size();
}

QVariant WatchlistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const WatchEntry &e = m_entries[index.row()];

    switch (role) {
    case SymbolRole:    return e.symbol;
    case NameRole:      return e.name;
    case PriceRole:     return e.price;
    case ChangeRole:    return e.price - e.prevClose;
    case ChangePctRole:
        if (e.prevClose > 0.0)
            return ((e.price - e.prevClose) / e.prevClose) * 100.0;
        return 0.0;
    }
    return {};
}

QHash<int, QByteArray> WatchlistModel::roleNames() const
{
    return {
        { SymbolRole,    "symbol"    },
        { NameRole,      "name"      },
        { PriceRole,     "price"     },
        { ChangeRole,    "change"    },
        { ChangePctRole, "changePct" }
    };
}

bool WatchlistModel::addStock(const QString &symbol, const QString &name)
{
    if (m_entries.size() >= 20) {
        qWarning() << "Watchlist full (max 20)";
        return false;
    }

    if (contains(symbol))
        return false;

    beginInsertRows({}, m_entries.size(), m_entries.size());
    m_entries.append({ symbol.toUpper(), name, 0.0, 0.0 });
    endInsertRows();
    saveWatchlist();

    emit countChanged();
    return true;
}

void WatchlistModel::removeStock(const QString &symbol)
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].symbol == symbol.toUpper()) {
            beginRemoveRows({}, i, i);
            m_entries.removeAt(i);
            endRemoveRows();
            saveWatchlist();

            emit countChanged();
            return;
        }
    }
}

bool WatchlistModel::contains(const QString &symbol) const
{
    const QString upper = symbol.toUpper();
    for (const auto &e : m_entries)
        if (e.symbol == upper)
            return true;
    return false;
}

QStringList WatchlistModel::symbols() const
{
    QStringList out;
    for (const auto &e : m_entries)
        out << e.symbol;
    return out;
}

void WatchlistModel::updatePrice(const QString &symbol, double price, double prevClose)
{
    const QString upper = symbol.toUpper();
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].symbol == upper) {
            m_entries[i].price = price;
            if (prevClose > 0.0)
                m_entries[i].prevClose = prevClose;

            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, { PriceRole, ChangeRole, ChangePctRole });
            return;
        }
    }
}

void WatchlistModel::saveWatchlist()
{
    QSettings settings("StockViewerQT", "StockViewerQT");
    qDebug() << "Settings file:" << settings.fileName();

    QStringList stocks;

    for (const auto &e : m_entries)
    {
        stocks << QString("%1|%2").arg(e.symbol).arg(e.name);
    }

    settings.setValue("watchlist", stocks);
}

void WatchlistModel::loadWatchlist()
{
    QSettings settings("StockViewerQT", "StockViewerQT");
    qDebug() << "Settings file:" << settings.fileName();

    QStringList stocks = settings.value("watchlist").toStringList();

    beginResetModel();

    m_entries.clear();

    for (const QString &item : stocks)
    {
        QStringList parts = item.split('|');

        if (parts.size() < 2)
            continue;

        WatchEntry e;
        e.symbol = parts[0];
        e.name = parts[1];

        m_entries.append(e);
    }

    endResetModel();

    emit countChanged();
}
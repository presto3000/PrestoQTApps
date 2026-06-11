#include "stockhistorymodel.h"

StockHistoryModel::StockHistoryModel(StockHistoryStore *store, QObject *parent)
    : QAbstractListModel(parent),
    m_store(store)
{
    connect(store, &StockHistoryStore::historyUpdated,
            this, &StockHistoryModel::onHistoryUpdated);
}

int StockHistoryModel::rowCount(const QModelIndex &) const
{
    return m_points.size();
}

QVariant StockHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_points.size())
        return {};

    const auto &p = m_points[index.row()];

    switch (role) {
    case TimeRole:  return p.time;
    case PriceRole: return p.price;
    case Sma20Role: return sma20At(index.row());
    case Sma50Role: return sma50At(index.row());
    case VolumeRole: return volumeAt(index.row());
    }

    return {};
}

QHash<int, QByteArray> StockHistoryModel::roleNames() const
{
    return {
        { TimeRole, "time" },
        { PriceRole, "price" },
        { Sma20Role, "sma20" },
        { Sma50Role, "sma50" },
        { VolumeRole, "volume" }
    };
}

QString StockHistoryModel::symbol() const
{
    return m_symbol;
}

void StockHistoryModel::setSymbol(const QString &symbol)
{
    if (m_symbol == symbol)
        return;

    m_symbol = symbol;
    m_points = m_store->history(symbol);

    beginResetModel();
    endResetModel();

    emit symbolChanged();
}

double StockHistoryModel::priceAt(int index) const
{
    if (index < 0 || index >= m_points.size())
        return 0.0;

    return m_points[index].price;
}

double StockHistoryModel::volumeAt(int index) const
{
    if (index < 0 || index >= m_points.size())
        return 0.0;
    return m_points[index].volume;
}

void StockHistoryModel::onHistoryUpdated(const QString &symbol)
{
    if (symbol != m_symbol)
        return;

    m_points = m_store->history(symbol);

    beginResetModel();
    endResetModel();
}

double StockHistoryModel::sma20At(int index) const
{
    return calculateSMA(index, 20);
}

double StockHistoryModel::sma50At(int index) const
{
    return calculateSMA(index, 50);
}

double StockHistoryModel::calculateSMA(int index, int period) const
{
    if (index < period - 1 || m_points.size() < period)
        return qQNaN();

    double sum = 0.0;
    for (int i = 0; i < period; ++i) {
        sum += m_points[index - i].price;
    }
    return sum / period;
}
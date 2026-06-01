#include "stockhistorystore.h"

StockHistoryStore::StockHistoryStore(QObject *parent)
    : QObject(parent)
{
}

void StockHistoryStore::addPrice(const QString &symbol, double price)
{
    auto &vec = m_data[symbol];

    vec.append({ QDateTime::currentDateTime(), price });

    if (vec.size() > 200)
        vec.removeFirst();

    emit historyUpdated(symbol);
}

QVector<PricePoint> StockHistoryStore::history(const QString &symbol) const
{
    return m_data.value(symbol);
}

void StockHistoryStore::setHistory(const QString &symbol,
                                   const QVector<PricePoint> &points)
{
    m_data[symbol] = points;
    emit historyUpdated(symbol);
}
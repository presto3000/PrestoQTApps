#ifndef STOCKHISTORYSTORE_H
#define STOCKHISTORYSTORE_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QDateTime>

struct PricePoint {
    QDateTime time;
    double price;
    double volume = 0.0;
};

class StockHistoryStore : public QObject
{
    Q_OBJECT

public:
    explicit StockHistoryStore(QObject *parent = nullptr);

    void addPrice(const QString &symbol, double price);
    QVector<PricePoint> history(const QString &symbol) const;

    void setHistory(const QString &symbol, const QVector<PricePoint> &points);
signals:
    void historyUpdated(const QString &symbol);

private:
    QHash<QString, QVector<PricePoint>> m_data;
};

#endif
#ifndef STOCKPROVIDER_H
#define STOCKPROVIDER_H

#include "StockModel.h"
#include "stockhistorystore.h"
#include <QString>


class IStockProvider
{
public:
    virtual ~IStockProvider() = default;

    virtual QString buildUrl(const QString &symbol) const = 0;

    virtual Stock parse(const QString &symbol,
                        const QByteArray &data) const = 0;

    virtual QString buildHistoryUrl(const QString& symbol) const = 0;
    virtual QVector<PricePoint> parseHistory(const QByteArray& data) const = 0;
};

class StooqProvider : public IStockProvider
{
public:
    QString buildUrl(const QString &symbol) const override;

    Stock parse(const QString &symbol,
                const QByteArray &data) const override;
    QString buildHistoryUrl(const QString &symbol) const override;
    QVector<PricePoint> parseHistory(const QByteArray &data) const override;
};

class YahooProvider : public IStockProvider
{
public:
    QString buildUrl(const QString &symbol) const override;

    Stock parse(const QString &symbol,
                const QByteArray &data) const override;
    QString buildHistoryUrl(const QString &symbol) const override;
    QVector<PricePoint> parseHistory(const QByteArray &data) const override;
};

#endif // STOCKPROVIDER_H

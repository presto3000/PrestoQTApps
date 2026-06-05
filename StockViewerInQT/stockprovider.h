#ifndef STOCKPROVIDER_H
#define STOCKPROVIDER_H

#include "StockModel.h"
#include "stockhistorystore.h"
#include <QNetworkRequest>
#include <QString>


class IStockProvider
{
public:
    virtual ~IStockProvider() = default;

    virtual QString buildUrl(const QString &symbol) const = 0;

    virtual Stock parse(const QString &symbol, const QByteArray &data) const = 0;
    virtual QString buildHistoryUrl(const QString& symbol) const = 0;
    virtual QVector<PricePoint> parseHistory(const QByteArray& data) const = 0;
};

class StooqProvider : public IStockProvider
{
public:
    QString buildUrl(const QString &symbol) const override;

    Stock parse(const QString &symbol, const QByteArray &data) const override;
    QString buildHistoryUrl(const QString &symbol) const override;
    QVector<PricePoint> parseHistory(const QByteArray &data) const override;
};

class YahooProvider : public IStockProvider
{
public:
    QString buildUrl(const QString &symbol) const override;

    Stock parse(const QString &symbol, const QByteArray &data) const override;
    QString buildHistoryUrl(const QString &symbol) const override;
    QVector<PricePoint> parseHistory(const QByteArray &data) const override;
};

class AlpacaProvider : public IStockProvider
{
public:
    void setCredentials(const QString &key, const QString &secret) { m_key = key; m_secret = secret; }

    QString buildUrl(const QString &symbol) const override;

    Stock   parse(const QString &symbol, const QByteArray &data) const override;
    QString buildHistoryUrl(const QString &symbol) const override;
    QVector<PricePoint> parseHistory(const QByteArray &data) const override;

    // Alpaca needs auth headers - expose them for QNetworkRequest
    void applyHeaders(QNetworkRequest &req) const;

private:
    QString m_key;
    QString m_secret;
};

#endif // STOCKPROVIDER_H

#include "stockprovider.h"

#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

QString StooqProvider::buildUrl(const QString &symbol) const
{
    return QString(
               "https://stooq.com/q/l/?s=%1.us&f=sd2t2ohlcv&e=json")
        .arg(symbol.toLower());
}

Stock StooqProvider::parse(const QString &symbol,
                           const QByteArray &data) const
{
    Stock s;
    s.symbol = symbol.toUpper();
    s.name = s.symbol;
    s.price = 0.0;

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
        return s;

    QJsonObject root = doc.object();
    QJsonArray arr = root["symbols"].toArray();

    if (arr.isEmpty())
        return s;

    auto obj = arr.first().toObject();

    s.name = obj["name"].toString(s.symbol);

    auto closeValue = obj["close"];

    if (closeValue.isDouble())
        s.price = closeValue.toDouble();
    else
        s.price = closeValue.toString().toDouble();

    qDebug() << "[StooqProvider] parse() called for:" << symbol;
    qDebug() << "[StooqProvider] raw size:" << data.size();

    return s;
}

QString StooqProvider::buildHistoryUrl(const QString &symbol) const
{
    return QString("https://stooq.com/q/d/l/?s=%1.us&i=d")
    .arg(symbol.toLower());
}

QVector<PricePoint> StooqProvider::parseHistory(const QByteArray &data) const
{
    QVector<PricePoint> out;

    QTextStream stream(data);
    QString line = stream.readLine(); // skip header

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        auto parts = line.split(',');
        if (parts.size() < 5)
            continue;

        QDateTime time = QDateTime::fromString(parts[0], "yyyy-MM-dd");
        double close = parts[4].toDouble();

        out.push_back({ time, close });
    }

    return out;
}

QString YahooProvider::buildUrl(const QString &symbol) const
{
    return QString(
               "https://query1.finance.yahoo.com/v8/finance/chart/%1")
        .arg(symbol);
}

Stock YahooProvider::parse(const QString &symbol,
                           const QByteArray &data) const
{
    Stock s;
    s.symbol = symbol.toUpper();
    s.name = s.symbol;
    s.price = 0.0;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return s;

    QJsonObject root = doc.object();

    QJsonObject chart = root["chart"].toObject();

    QJsonArray results = chart["result"].toArray();

    if (results.isEmpty())
        return s;

    QJsonObject result = results.first().toObject();

    // symbol
    QJsonObject meta = result["meta"].toObject();

    QString longName = meta["longName"].toString();
    QString shortName = meta["shortName"].toString();

    if (!longName.isEmpty())
        s.name = longName;
    else if (!shortName.isEmpty())
        s.name = shortName;

    // current market price
    if (meta["regularMarketPrice"].isDouble())
        s.price = meta["regularMarketPrice"].toDouble();

    qDebug() << "[YahooProvider] parse() called for:" << symbol;
    qDebug() << "[YahooProvider] raw size:" << data.size();

    return s;
}

QString YahooProvider::buildHistoryUrl(const QString &symbol) const
{
    return QString(
               "https://query1.finance.yahoo.com/v8/finance/chart/%1?interval=1d&range=1y"
               ).arg(symbol);
}

QVector<PricePoint> YahooProvider::parseHistory(const QByteArray &data) const
{
    QVector<PricePoint> result;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "Yahoo parseHistory: invalid JSON";
        return result;
    }

    QJsonObject root = doc.object();

    QJsonValue chartValue = root.value("chart");
    if (!chartValue.isObject())
        return result;

    QJsonObject chart = chartValue.toObject();

    QJsonArray results = chart.value("result").toArray();
    if (results.isEmpty())
        return result;

    QJsonObject resultObj = results.first().toObject();

    QJsonArray timestamps = resultObj.value("timestamp").toArray();

    QJsonObject indicators =
        resultObj.value("indicators").toObject()
            .value("quote").toArray().first().toObject();

    QJsonArray close = indicators.value("close").toArray();

    int n = qMin(timestamps.size(), close.size());

    result.reserve(n);

    for (int i = 0; i < n; i++)
    {
        if (close[i].isNull())
            continue;

        PricePoint p;
        p.time = QDateTime::fromSecsSinceEpoch(timestamps[i].toInteger());
        p.price = close[i].toDouble();

        result.append(p);
    }

    return result;
}
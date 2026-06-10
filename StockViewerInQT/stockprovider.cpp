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
    s.prevClose = 0.0;

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

    // Previous close
    auto prevCloseValue = obj["previousClose"];   // Stooq field
    if (prevCloseValue.isDouble())
        s.prevClose = prevCloseValue.toDouble();
    else if (!prevCloseValue.toString().isEmpty())
        s.prevClose = prevCloseValue.toString().toDouble();

    qDebug() << "[StooqProvider] parse() called for:" << symbol;
    // qDebug() << "[StooqProvider] raw size:" << data.size();

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
    s.prevClose = 0.0;

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

    // Previous close - Yahoo has this field
    if (meta["previousClose"].isDouble())
        s.prevClose = meta["previousClose"].toDouble();

    qDebug() << "[YahooProvider] parse() called for:" << symbol;
    // qDebug() << "[YahooProvider] raw size:" << data.size();

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

void AlpacaProvider::applyHeaders(QNetworkRequest &req) const
{
    req.setRawHeader("APCA-API-KEY-ID",     m_key.toUtf8());
    req.setRawHeader("APCA-API-SECRET-KEY", m_secret.toUtf8());
}

QString AlpacaProvider::buildUrl(const QString &symbol) const
{
    // Latest trade endpoint
    return QString("https://data.alpaca.markets/v2/stocks/%1/trades/latest")
        .arg(symbol.toUpper());
}

Stock AlpacaProvider::parse(const QString &symbol, const QByteArray &data) const
{
    Stock s;
    s.symbol = symbol.toUpper();
    s.name   = s.symbol;
    s.price  = 0.0;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return s;

    // { "trade": { "p": 182.5, ... } }
    QJsonObject trade = doc.object()["trade"].toObject();
    s.price = trade["p"].toDouble();

    qDebug() << "[AlpacaProvider] parse()" << symbol << "price:" << s.price;
    return s;
}

QString AlpacaProvider::buildHistoryUrl(const QString &symbol) const
{
    // 1 year of daily bars
    QDate from = QDate::currentDate().addYears(-1);
    return QString(
               "https://data.alpaca.markets/v2/stocks/%1/bars"
               "?timeframe=1Day&start=%2&limit=365&feed=iex"
               ).arg(symbol.toUpper(), from.toString("yyyy-MM-dd"));
}

QVector<PricePoint> AlpacaProvider::parseHistory(const QByteArray &data) const
{
    QVector<PricePoint> result;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return result;

    QJsonArray bars = doc.object()["bars"].toArray();
    result.reserve(bars.size());

    for (const QJsonValue &v : bars) {
        QJsonObject bar = v.toObject();
        PricePoint  p;
        p.time  = QDateTime::fromString(bar["t"].toString(), Qt::ISODate);
        p.price = bar["c"].toDouble();   // close price
        if (p.price > 0.0)
            result.append(p);
    }

    qDebug() << "[AlpacaProvider] parseHistory() bars:" << result.size();
    return result;
}

// =============================================================================
// FinnhubProvider
// =============================================================================

void FinnhubProvider::applyHeaders(QNetworkRequest &req) const
{
    req.setRawHeader("X-Finnhub-Token", m_key.toUtf8());
}

// Quote endpoint: https://finnhub.io/api/v1/quote?symbol=AAPL
QString FinnhubProvider::buildUrl(const QString &symbol) const
{
    return QString("https://finnhub.io/api/v1/quote?symbol=%1&token=%2").arg(symbol.toUpper(), m_key);
}

Stock FinnhubProvider::parse(const QString &symbol, const QByteArray &data) const
{
    Stock s;
    s.symbol = symbol.toUpper();
    s.name   = s.symbol;
    s.price  = 0.0;
    s.prevClose = 0.0;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[FinnhubProvider] JSON parse error for" << symbol << ":" << err.errorString();
        return s;
    }

    // { "c": current, "h": high, "l": low, "o": open,
    //   "pc": prev close, "t": timestamp }
    QJsonObject obj = doc.object();

    s.price     = obj["c"].toDouble();
    s.prevClose = obj["pc"].toDouble();

    qDebug() << "[FinnhubProvider] parse()" << symbol
             << "price:" << s.price << "prevClose:" << s.prevClose;
    return s;
}

// Candle endpoint - 1 year of daily bars
// https://finnhub.io/api/v1/stock/candle?symbol=AAPL&resolution=D&from=...&to=...
// QString FinnhubProvider::buildHistoryUrl(const QString &symbol) const
// {
//     const qint64 to   = QDateTime::currentSecsSinceEpoch();
//     const qint64 from = to - 365LL * 24 * 3600;   // 1 year back
//
//     return QString("https://finnhub.io/api/v1/stock/candle""?symbol=%1&resolution=D&from=%2&to=%3&token=%4")
//         .arg(symbol.toUpper())
//         .arg(from)
//         .arg(to)
//         .arg(m_key);
// }

// History: /stock/candle requires a paid Finnhub plan.
// Yahoo provider instead
QString FinnhubProvider::buildHistoryUrl(const QString &symbol) const
{
    // Exactly the same URL YahooProvider uses
    return QString(
               "https://query1.finance.yahoo.com/v8/finance/chart/%1?interval=1d&range=1y"
               ).arg(symbol.toUpper());
}

QVector<PricePoint> FinnhubProvider::parseHistory(const QByteArray &data) const
{
    // Identical parsing to YahooProvider::parseHistory — same endpoint, same JSON shape
    QVector<PricePoint> result;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[FinnhubProvider] parseHistory: invalid JSON";
        return result;
    }

    QJsonObject root = doc.object();

    QJsonValue chartValue = root.value("chart");
    if (!chartValue.isObject())
        return result;

    QJsonArray results = chartValue.toObject().value("result").toArray();
    if (results.isEmpty())
        return result;

    QJsonObject resultObj = results.first().toObject();

    QJsonArray timestamps = resultObj.value("timestamp").toArray();

    QJsonObject indicators =
        resultObj.value("indicators").toObject()
            .value("quote").toArray().first().toObject();

    QJsonArray close = indicators.value("close").toArray();

    const int n = qMin(timestamps.size(), close.size());
    result.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (close[i].isNull())
            continue;
        PricePoint p;
        p.time  = QDateTime::fromSecsSinceEpoch(timestamps[i].toInteger());
        p.price = close[i].toDouble();
        result.append(p);
    }

    qDebug() << "[FinnhubProvider] parseHistory() points:" << result.size();
    return result;
}
#include "alpacapositionprovider.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AlpacaPositionProvider::AlpacaPositionProvider(QObject *parent)
    : IPositionProvider(parent)
{}

void AlpacaPositionProvider::setCredentials(const QString &key, const QString &secret, bool paper)
{
    m_key    = key;
    m_secret = secret;
    m_paper  = paper;
}

QString AlpacaPositionProvider::baseUrl() const
{
    return m_paper ? "https://paper-api.alpaca.markets" : "https://api.alpaca.markets";
}

QNetworkRequest AlpacaPositionProvider::makeRequest(const QString &path) const
{
    QNetworkRequest req(QUrl(baseUrl() + path));
    req.setRawHeader("APCA-API-KEY-ID",     m_key.toUtf8());
    req.setRawHeader("APCA-API-SECRET-KEY", m_secret.toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return req;
}

// -- fetchPositions ---------------------------------------------------------

void AlpacaPositionProvider::fetchPositions()
{
    if (m_key.isEmpty()) {
        emit fetchError("No credentials set");
        return;
    }

    // qDebug() << "[AlpacaPositions] Fetching from" << name();

    auto *reply = m_manager.get(makeRequest("/v2/positions"));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QString err = reply->errorString();
            qWarning() << "[AlpacaPositions] fetch error:" << err;
            emit fetchError(err);
            return;
        }

        const QByteArray data = reply->readAll();
        const QJsonArray arr  = QJsonDocument::fromJson(data).array();

        QList<Position> positions;
        positions.reserve(arr.size());

        for (const QJsonValue &v : arr) {
            const QJsonObject obj = v.toObject();

            Position p;
            p.symbol           = obj["symbol"].toString();
            p.qty              = obj["qty"].toString().toDouble();
            p.avgEntryPrice    = obj["avg_entry_price"].toString().toDouble();
            p.currentPrice     = obj["current_price"].toString().toDouble();
            p.marketValue      = obj["market_value"].toString().toDouble();
            p.unrealizedPL     = obj["unrealized_pl"].toString().toDouble();
            p.unrealizedPLPct  = obj["unrealized_plpc"].toString().toDouble() * 100.0;
            p.costBasis        = obj["cost_basis"].toString().toDouble();
            p.side             = obj["side"].toString();   // "long" | "short"
            p.name             = p.symbol;  // Alpaca positions don't include company name

            positions.append(p);
        }

        // qDebug() << "[AlpacaPositions] Got" << positions.size() << "positions";
        emit positionsFetched(positions);
    });
}

// -- closePosition ----------------------------------------------------------

void AlpacaPositionProvider::closePosition(const QString &symbol)
{
    if (m_key.isEmpty()) {
        emit fetchError("No credentials set");
        return;
    }

    qDebug() << "[AlpacaPositions] Closing position:" << symbol;

    // DELETE /v2/positions/{symbol} — Alpaca liquidates at market
    auto *reply = m_manager.deleteResource(makeRequest("/v2/positions/" + symbol.toUpper()));

    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        reply->deleteLater();

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() != QNetworkReply::NoError) {
            const QString err = reply->errorString();
            qWarning() << "[AlpacaPositions] close error for" << symbol << ":" << err;
            emit positionClosed(symbol, false, err);
            return;
        }

        qDebug() << "[AlpacaPositions] Closed" << symbol << "HTTP" << status;
        emit positionClosed(symbol, true, {});

        // Re-fetch so the model reflects the closed position
        fetchPositions();
    });
}
#include "alpacawebsocket.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

// Alpaca free tier uses iex feed; paid uses sip
static constexpr char WS_URL[] = "wss://stream.data.alpaca.markets/v2/iex";

AlpacaWebSocket::AlpacaWebSocket(WatchlistModel *watchlist, QObject *parent)
    : QObject(parent),
    m_watchlist(watchlist)
{
    connect(&m_socket, &QWebSocket::connected,
            this,      &AlpacaWebSocket::onConnected);

    connect(&m_socket, &QWebSocket::disconnected,
            this,      &AlpacaWebSocket::onDisconnected);

    connect(&m_socket, &QWebSocket::textMessageReceived,
            this,      &AlpacaWebSocket::onMessageReceived);

    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &AlpacaWebSocket::onError);

    // Reconnect timer — fires once after delay
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout,
            this, &AlpacaWebSocket::connectToFeed);

    // Ping every 20s to keep connection alive
    connect(&m_pingTimer, &QTimer::timeout, this, [this]() {
        if (m_socket.state() == QAbstractSocket::ConnectedState)
            m_socket.ping();
    });
    m_pingTimer.start(20000);

    // Re-subscribe whenever watchlist changes
    connect(m_watchlist, &WatchlistModel::countChanged,
            this,        &AlpacaWebSocket::subscribeAll);
}

void AlpacaWebSocket::setCredentials(const QString &key, const QString &secret)
{
    m_key    = key;
    m_secret = secret;
}

void AlpacaWebSocket::connectToFeed()
{
    if (m_key.isEmpty() || m_secret.isEmpty()) {
        qWarning() << "[AlpacaWS] No credentials set — not connecting";
        return;
    }

    qDebug() << "[AlpacaWS] Connecting to" << WS_URL;
    m_socket.open(QUrl(QString::fromLatin1(WS_URL)));
}

void AlpacaWebSocket::disconnect()
{
    m_reconnectTimer.stop();
    m_pingTimer.stop();
    m_socket.close();
}

// -- Slots ---------------------------------------------------------------------

void AlpacaWebSocket::onConnected()
{
    qDebug() << "[AlpacaWS] Connected — authenticating";
    m_reconnectDelay = 3000;   // reset backoff

    // Authenticate
    QJsonObject auth;
    auth["action"] = "auth";
    auth["key"]    = m_key;
    auth["secret"] = m_secret;
    sendJson(auth);
}

void AlpacaWebSocket::onDisconnected()
{
    qDebug() << "[AlpacaWS] Disconnected";
    m_authenticated = false;
    emit connectedChanged();
    attemptReconnect();
}

void AlpacaWebSocket::onError(QAbstractSocket::SocketError err)
{
    qWarning() << "[AlpacaWS] Socket error:" << err << m_socket.errorString();
    emit error(m_socket.errorString());
    attemptReconnect();
}

void AlpacaWebSocket::onMessageReceived(const QString &msg)
{
    // Alpaca sends arrays of messages
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());

    QJsonArray arr = doc.isArray()
                         ? doc.array()
                         : QJsonArray{ doc.object() };

    for (const QJsonValue &val : arr) {
        const QJsonObject obj = val.toObject();
        const QString     T   = obj["T"].toString();   // message type

        if (T == "success") {
            const QString msg = obj["msg"].toString();
            qDebug() << "[AlpacaWS] success:" << msg;

            if (msg == "authenticated") {
                m_authenticated = true;
                emit connectedChanged();
                subscribeAll();
            }

        } else if (T == "error") {
            qWarning() << "[AlpacaWS] server error:" << obj["msg"].toString();
            emit error(obj["msg"].toString());

        } else if (T == "subscription") {
            qDebug() << "[AlpacaWS] Subscriptions confirmed:"
                     << obj["trades"].toArray().count() << "trades,"
                     << obj["quotes"].toArray().count() << "quotes";

        } else if (T == "t") {
            handleTrade(obj);

        } else if (T == "q") {
            handleQuote(obj);
        }
    }
}

// -- Handlers ------------------------------------------------------------------

void AlpacaWebSocket::handleTrade(const QJsonObject &obj)
{
    // Trade message: { T:"t", S:"AAPL", p:182.5, s:100, t:"2024-..." }
    const QString symbol = obj["S"].toString();
    const double  price  = obj["p"].toDouble();

    if (symbol.isEmpty() || price <= 0.0)
        return;

    qDebug() << "[AlpacaWS] Trade" << symbol << price;
    m_watchlist->updatePrice(symbol, price, 0.0);
}

void AlpacaWebSocket::handleQuote(const QJsonObject &obj)
{
    // Quote message: { T:"q", S:"AAPL", ap:182.6, bp:182.4, ... }
    const QString symbol = obj["S"].toString();
    // Use mid of ask/bid as price
    const double ask = obj["ap"].toDouble();
    const double bid = obj["bp"].toDouble();

    if (symbol.isEmpty() || (ask <= 0.0 && bid <= 0.0))
        return;

    double mid = (ask > 0.0 && bid > 0.0)
                     ? (ask + bid) / 2.0
                     : qMax(ask, bid);

    m_watchlist->updatePrice(symbol, mid, 0.0);
}

void AlpacaWebSocket::subscribeAll()
{
    if (!m_authenticated)
        return;

    const QStringList syms = m_watchlist->symbols();

    if (syms.isEmpty())
        return;

    QJsonArray symbolsJson;
    for (const QString &s : syms)
        symbolsJson.append(s);

    // Subscribe to trades (more reliable than quotes for "last price")
    QJsonObject sub;
    sub["action"] = "subscribe";
    sub["trades"] = symbolsJson;
    sub["quotes"] = symbolsJson;

    sendJson(sub);
    qDebug() << "[AlpacaWS] Subscribed to" << syms;
}

// -- Helpers -------------------------------------------------------------------

void AlpacaWebSocket::sendJson(const QJsonObject &obj)
{
    const QString text = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact)
        );
    m_socket.sendTextMessage(text);
}

void AlpacaWebSocket::attemptReconnect()
{
    if (m_key.isEmpty() || m_secret.isEmpty())
        return;

    qDebug() << "[AlpacaWS] Reconnecting in" << m_reconnectDelay << "ms";
    m_reconnectTimer.start(m_reconnectDelay);

    // Exponential backoff, cap at 30s
    m_reconnectDelay = qMin(m_reconnectDelay * 2, 30000);
}
#include "finnhubwebsocket.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

// wss://ws.finnhub.io?token=<key>
static const QString WS_BASE = "wss://ws.finnhub.io";

FinnhubWebSocket::FinnhubWebSocket(WatchlistModel *watchlist, TradeTickModel *tapeTicks, QObject *parent)
    : QObject(parent),
    m_watchlist(watchlist),
    m_tapeTicks(tapeTicks)
{
    // --- Socket signals ---
    connect(&m_socket, &QWebSocket::connected,
            this,      &FinnhubWebSocket::onConnected);

    connect(&m_socket, &QWebSocket::disconnected,
            this,      &FinnhubWebSocket::onDisconnected);

    connect(&m_socket, &QWebSocket::textMessageReceived,
            this,      &FinnhubWebSocket::onMessageReceived);

    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &FinnhubWebSocket::onSocketError);

    // --- Reconnect timer (single-shot) ---
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout,
            this,              &FinnhubWebSocket::connectToFeed);

    // --- Keepalive ping every 20 s ---
    connect(&m_pingTimer, &QTimer::timeout, this, [this]() {
        if (m_socket.state() == QAbstractSocket::ConnectedState)
            m_socket.ping();
    });
    m_pingTimer.start(20'000);

    // --- Re-subscribe when watchlist changes ---
    connect(m_watchlist, &WatchlistModel::countChanged,
            this,        &FinnhubWebSocket::subscribeAll);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void FinnhubWebSocket::setApiKey(const QString &key)
{
    m_key = key;
}

void FinnhubWebSocket::connectToFeed()
{
    if (m_key.isEmpty()) {
        qWarning() << "[FinnhubWS] No API key set — not connecting";
        return;
    }

    if (m_socket.state() == QAbstractSocket::ConnectedState ||
        m_socket.state() == QAbstractSocket::ConnectingState) {
        qDebug() << "[FinnhubWS] Already connecting/connected";
        return;
    }

    const QUrl url(WS_BASE + "?token=" + m_key);
    qDebug() << "[FinnhubWS] Connecting to" << WS_BASE;
    m_socket.open(url);
}

void FinnhubWebSocket::disconnectFeed()
{
    m_reconnectTimer.stop();
    m_pingTimer.stop();
    m_socket.close();
}

void FinnhubWebSocket::subscribeAll()
{
    if (!m_connected)
        return;

    const QStringList current = m_watchlist->symbols();
    const QSet<QString> wanted(current.begin(), current.end());

    // Unsubscribe symbols removed from watchlist
    for (const QString &sym : m_subscribed) {
        if (!wanted.contains(sym))
            unsubscribe(sym);
    }

    // Subscribe new symbols
    for (const QString &sym : wanted) {
        if (!m_subscribed.contains(sym))
            subscribe(sym);
    }
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void FinnhubWebSocket::onConnected()
{
    qDebug() << "[FinnhubWS] Connected";
    m_connected = true;
    m_reconnectDelay = 3000;
    m_subscribed.clear();   // socket is fresh - no active subs yet

    emit connectedChanged();
    subscribeAll();
}

void FinnhubWebSocket::onDisconnected()
{
    qDebug() << "[FinnhubWS] Disconnected";
    m_connected = false;
    m_subscribed.clear();
    emit connectedChanged();
    attemptReconnect();
}

void FinnhubWebSocket::onSocketError(QAbstractSocket::SocketError err)
{
    qWarning() << "[FinnhubWS] Socket error:" << err << m_socket.errorString();
    emit errorOccurred(m_socket.errorString());
    // onDisconnected will fire next and trigger reconnect
}

void FinnhubWebSocket::onMessageReceived(const QString &msg)
{
    // Finnhub sends a single top-level object, not an array
    const QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    if (!doc.isObject())
        return;

    const QJsonObject obj  = doc.object();
    const QString     type = obj["type"].toString();

    if (type == "trade") {
        // data is an array of trade objects
        const QJsonArray trades = obj["data"].toArray();
        for (const QJsonValue &v : trades) {
            const QJsonObject t = v.toObject();

            const QString symbol = t["s"].toString();
            const double  price  = t["p"].toDouble();
            const double  volume = t["v"].toDouble();   // Finnhub sends fractional shares
            const qint64  tsMs   = static_cast<qint64>(t["t"].toDouble()); // epoch ms

            if (symbol.isEmpty() || price <= 0.0)
                continue;

            // Condition codes - array of strings, e.g. ["1"] = Regular
            // We don't filter by condition on free tier (too noisy to be worth it)

            // Finnhub free WS has no bid/ask, so we can't determine side reliably.
            // Mark as empty so the badge is hidden rather than wrong.
            TradeTick tick;
            tick.time     = QDateTime::fromMSecsSinceEpoch(tsMs);
            tick.symbol   = symbol;
            tick.price    = price;
            tick.size     = static_cast<int>(volume);
            tick.side     = "";           // not determinable without quote stream
            tick.exchange = "FINNHUB";    // Finnhub aggregates multiple venues

            m_tapeTicks->addTick(tick);

            // Keep watchlist price up to date
            m_watchlist->updatePrice(symbol, price, 0.0);
        }

    } else if (type == "ping") {
        // Server-initiated ping — reply with pong
        QJsonObject pong;
        pong["type"] = "pong";
        sendJson(pong);

    } else if (type == "error") {
        const QString errMsg = obj["msg"].toString();
        qWarning() << "[FinnhubWS] Server error:" << errMsg;
        emit errorOccurred(errMsg);

    } else {
        // "ping", subscription acks, etc. — log once
        qDebug() << "[FinnhubWS] msg type:" << type;
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void FinnhubWebSocket::subscribe(const QString &symbol)
{
    QJsonObject sub;
    sub["type"]   = "subscribe";
    sub["symbol"] = symbol;
    sendJson(sub);
    m_subscribed.insert(symbol);
    qDebug() << "[FinnhubWS] Subscribed:" << symbol;
}

void FinnhubWebSocket::unsubscribe(const QString &symbol)
{
    QJsonObject unsub;
    unsub["type"]   = "unsubscribe";
    unsub["symbol"] = symbol;
    sendJson(unsub);
    m_subscribed.remove(symbol);
    qDebug() << "[FinnhubWS] Unsubscribed:" << symbol;
}

void FinnhubWebSocket::sendJson(const QJsonObject &obj)
{
    const QString text = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket.sendTextMessage(text);
}

void FinnhubWebSocket::attemptReconnect()
{
    if (m_key.isEmpty())
        return;

    qDebug() << "[FinnhubWS] Reconnecting in" << m_reconnectDelay << "ms";
    m_reconnectTimer.start(m_reconnectDelay);

    // Exponential backoff, cap at 30 s
    m_reconnectDelay = qMin(m_reconnectDelay * 2, 30'000);
}
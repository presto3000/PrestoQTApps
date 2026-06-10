#ifndef FINNHUBWEBSOCKET_H
#define FINNHUBWEBSOCKET_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QSet>
#include <QAbstractSocket>
#include "watchlistmodel.h"
#include "tradetickmodel.h"

// ---------------------------------------------------------------------------
// FinnhubWebSocket
//
// Connects to wss://ws.finnhub.io?token=<key>
//
// Notes:
//  - Free tier: 50 symbols - max.
// ---------------------------------------------------------------------------

class FinnhubWebSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit FinnhubWebSocket(WatchlistModel *watchlist, TradeTickModel *tapeTicks, QObject *parent = nullptr);

    void setApiKey(const QString &key);

    // Opens the WebSocket connection. Safe to call repeatedly.
    void connectToFeed();

    // Graceful shutdown — stops reconnect timer.
    void disconnectFeed();

    // Re-subscribe all current watchlist symbols. Called automatically
    // when the watchlist changes or after (re)authentication.
    void subscribeAll();

    bool isConnected() const { return m_connected; }

signals:
    void connectedChanged();
    void errorOccurred(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QString &msg);
    void onSocketError(QAbstractSocket::SocketError err);

private:
    void sendJson(const QJsonObject &obj);
    void subscribe(const QString &symbol);
    void unsubscribe(const QString &symbol);
    void attemptReconnect();

    QWebSocket     m_socket;
    WatchlistModel *m_watchlist;
    TradeTickModel *m_tapeTicks;
    QTimer          m_reconnectTimer;
    QTimer          m_pingTimer;

    QString  m_key;
    bool     m_connected     = false;
    int      m_reconnectDelay = 3000;   // ms; doubles up to 30 s

    // Track which symbols are currently subscribed so we can
    // diff on watchlist change and only send delta messages.
    QSet<QString> m_subscribed;
};
#endif // FINNHUBWEBSOCKET_H

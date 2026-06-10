#ifndef ALPACAWEBSOCKET_H
#define ALPACAWEBSOCKET_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QStringList>
#include <QAbstractSocket>
#include "watchlistmodel.h"
#include "tradetickmodel.h"

struct QuoteSnapshot {
    double bid = 0.0;
    double ask = 0.0;
    int bidSize = 0;
    int askSize = 0;
};

class AlpacaWebSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit AlpacaWebSocket(WatchlistModel  *watchlist,
                             TradeTickModel  *tapeTicks,
                             QObject         *parent = nullptr);

    void setCredentials(const QString &key, const QString &secret);
    void connectToFeed();
    void disconnect();

    // Update subscription when watchlist changes
    void subscribeAll();

    bool isConnected() const { return m_authenticated; }

signals:
    void connectedChanged();
    void error(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QString &msg);
    void onError(QAbstractSocket::SocketError err);

private:
    void sendJson(const QJsonObject &obj);
    void handleAuthMessage(const QJsonObject &obj);
    void handleQuote(const QJsonObject &obj);
    void handleTrade(const QJsonObject &obj);
    void attemptReconnect();

    QWebSocket      m_socket;
    WatchlistModel  *m_watchlist;
    TradeTickModel  *m_tapeTicks;
    QTimer          m_reconnectTimer;
    QTimer          m_pingTimer;

    QString m_key;
    QString m_secret;

    bool m_authenticated = false;
    int  m_reconnectDelay = 3000;   // ms, doubles on each failure up to 30s

    QHash<QString, QuoteSnapshot> m_quotes;
    QHash<QString, QString>       m_lastSide;  // last known trade direction per symbol
};

#endif // ALPACAWEBSOCKET_H

#ifndef STOCKFETCHER_H
#define STOCKFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QStringList>
#include <StockHistoryStore.h>
#include "StockModel.h"
#include "stockprovider.h"
#include "watchlistmodel.h"

class StockFetcher : public QObject
{
    Q_OBJECT
public:

    explicit StockFetcher(WatchlistModel *watchlist, StockHistoryStore *historyStore, QObject *parent = nullptr);

    // Start periodic refresh of watchlist prices (default: every 30s)
    void start(int intervalMs = 30000);
    void stop();

    void fetch(const QStringList &symbols);

    Q_INVOKABLE void setProvider(int index);

    void setAlpacaCredentials(const QString &key, const QString &secret);
    void setFinnhubApiKey(const QString &key);

    Q_INVOKABLE void fetchHistory(const QString &symbol);

    // Fetch prices for all current watchlist symbols right now
    Q_INVOKABLE void refreshNow();

signals:
    void providerChanged();

private slots:

private:

    void fetchPrice(const QString &symbol);

    QNetworkAccessManager m_manager;

    std::unique_ptr<IStockProvider> m_provider;

    QString m_alpacaKey;
    QString m_alpacaSecret;
    QString m_finnhubKey;

    QTimer m_timer;
    WatchlistModel       *m_watchlist;
    StockHistoryStore *m_historyStore;
};

#endif // STOCKFETCHER_H

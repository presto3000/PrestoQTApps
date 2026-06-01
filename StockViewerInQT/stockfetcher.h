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
    void setProvider(bool useYahoo);

    Q_INVOKABLE void setProvider(int index);
    Q_INVOKABLE void fetchHistory(const QString &symbol);

    // Fetch prices for all current watchlist symbols right now
    Q_INVOKABLE void refreshNow();

signals:
    void providerChanged();

private slots:

private:

    void fetchPrice(const QString &symbol);

    QNetworkAccessManager m_manager;
    StockModel *m_model;


    QStringList m_symbols;

    QList<Stock> m_cache;
    int m_pendingReplies = 0;

    QStringList m_allSymbols;
    int m_batchSize = 0;
    int m_currentIndex = 0;

    std::unique_ptr<IStockProvider> m_provider;

    QTimer m_timer;
    WatchlistModel       *m_watchlist;
    StockHistoryStore *m_historyStore;
};

#endif // STOCKFETCHER_H

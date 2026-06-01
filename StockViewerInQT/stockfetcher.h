#ifndef STOCKFETCHER_H
#define STOCKFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QStringList>
#include <StockHistoryStore.h>
#include "StockModel.h"
#include "stockprovider.h"

class StockFetcher : public QObject
{
    Q_OBJECT
public:

    explicit StockFetcher(StockModel *model, StockHistoryStore *historyStore = nullptr, QObject *parent = nullptr);


    void start(const QStringList &symbols, int intervalMs = 5000);
    void stop();

    void fetch(const QStringList &symbols);

    void finishPendingReply();
    void startBatched(const QStringList &allSymbols, int batchSize = 100, int intervalMs = 5000);

    void fetchNextBatch();

    QString buildUrl(const QString &symbol) const;
    Stock parseReply(const QString &symbol, const QJsonDocument &doc) const;


    void setProvider(bool useYahoo);

    Q_INVOKABLE void setProvider(int index);

    Q_INVOKABLE void fetchHistory(const QString &symbol);
signals:
    void providerChanged();

private slots:
    // void onReplyFinished();


private:
    QNetworkAccessManager m_manager;
    StockModel *m_model;

    QTimer m_timer;
    QStringList m_symbols;

    QList<Stock> m_cache;
    int m_pendingReplies = 0;

    QStringList m_allSymbols;
    int m_batchSize = 0;
    int m_currentIndex = 0;

    std::unique_ptr<IStockProvider> m_provider;
    StockHistoryStore *m_historyStore;
};

#endif // STOCKFETCHER_H

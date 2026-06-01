#include "stockfetcher.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <algorithm>

StockFetcher::StockFetcher(WatchlistModel *watchlist,
                           StockHistoryStore *historyStore,
                           QObject *parent)
    : QObject(parent),
    m_watchlist(watchlist),
    m_historyStore(historyStore),
    m_provider(std::make_unique<StooqProvider>())
{
    connect(&m_timer, &QTimer::timeout, this, &StockFetcher::refreshNow);
}

void StockFetcher::start(int intervalMs)
{
    m_timer.start(intervalMs);
    refreshNow();
}

void StockFetcher::stop()
{
    m_timer.stop();
}

void StockFetcher::fetchPrice(const QString &symbol)
{
    QNetworkRequest request{QUrl(m_provider->buildUrl(symbol))};
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Price fetch error for" << symbol << ":" << reply->errorString();
            return;
        }

        Stock s = m_provider->parse(symbol, reply->readAll());

        if (s.price <= 0.0) {
            qWarning() << "Invalid price for" << symbol;
            return;
        }

        // prevClose: providers don't always supply it — pass 0 and let the model keep its old value
        m_watchlist->updatePrice(symbol, s.price, s.prevClose);
    });
}

void StockFetcher::fetchHistory(const QString &symbol)
{
    QNetworkRequest req{QUrl(m_provider->buildHistoryUrl(symbol))};
    req.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

    QNetworkReply *reply = m_manager.get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "History fetch error for" << symbol << ":" << reply->errorString();
            return;
        }

        auto series = m_provider->parseHistory(reply->readAll());
        m_historyStore->setHistory(symbol, series);
    });
}

void StockFetcher::setProvider(bool useYahoo)
{
    if (useYahoo)
        m_provider = std::make_unique<YahooProvider>();
    else
        m_provider = std::make_unique<StooqProvider>();
}


void StockFetcher::setProvider(int index)
{
    if (index == 1)
        m_provider = std::make_unique<YahooProvider>();
    else
        m_provider = std::make_unique<StooqProvider>();

    qDebug() << "Provider switched to index:" << index;
    emit providerChanged();

    // Re-fetch immediately with new provider
    refreshNow();
}

void StockFetcher::refreshNow()
{
    const QStringList syms = m_watchlist->symbols();
    if (syms.isEmpty())
        return;

    qDebug() << "Refreshing" << syms.size() << "watchlist stocks";

    for (const QString &symbol : syms)
        fetchPrice(symbol);
}



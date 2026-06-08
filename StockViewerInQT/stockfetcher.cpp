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
    request.setHeader(
        QNetworkRequest::UserAgentHeader,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/122.0.0.0 Safari/537.36"
        );

    // Apply Alpaca auth headers if needed
    if (auto *ap = dynamic_cast<AlpacaProvider*>(m_provider.get()))
        ap->applyHeaders(request);

    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        // Safely read everything FIRST
        const QByteArray data = reply->readAll();
        const auto error = reply->error();
        const int httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        reply->deleteLater();

        // Network error
        if (error != QNetworkReply::NoError) {
            qWarning() << "Price fetch error for" << symbol << ":"
                       << reply->errorString();
            return;
        }

        // HTTP error (403, 404, 500, etc.)
        if (httpStatus != 200) {
            qWarning() << "HTTP error for" << symbol << ":"
                       << httpStatus;
            qWarning() << "Response preview:" << data.left(200);
            return;
        }

        // Let provider parse safely
        Stock s = m_provider->parse(symbol, data);

        if (s.price <= 0.0 || std::isnan(s.price)) {
            qWarning() << "Invalid price for" << symbol
                       << "raw data:" << data.left(200);
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

    if (auto *ap = dynamic_cast<AlpacaProvider*>(m_provider.get()))
        ap->applyHeaders(req);

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

void StockFetcher::setProvider(int index)
{
    if (index == 1) {
        m_provider = std::make_unique<YahooProvider>();
    } else if (index == 2) {
        auto p = std::make_unique<AlpacaProvider>();
        p->setCredentials(m_alpacaKey, m_alpacaSecret);
        m_provider = std::move(p);
    } else {
        m_provider = std::make_unique<StooqProvider>();
    }

    qDebug() << "Provider switched to index:" << index;
    emit providerChanged();
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

void StockFetcher::setAlpacaCredentials(const QString &key, const QString &secret)
{
    m_alpacaKey    = key;
    m_alpacaSecret = secret;
}

#include "stockfetcher.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <algorithm>

StockFetcher::StockFetcher(StockModel *model, StockHistoryStore *historyStore, QObject *parent)
    : QObject(parent),
    m_model(model),
    m_provider(std::make_unique<StooqProvider>()),
    m_historyStore(historyStore)
{
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        fetch(m_symbols);
    });
}

void StockFetcher::fetchHistory(const QString &symbol)
{
    QString url = m_provider->buildHistoryUrl(symbol);

    QNetworkRequest req(url);
    auto reply = m_manager.get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        QByteArray data = reply->readAll();
        reply->deleteLater();

        auto series = m_provider->parseHistory(data);

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
    // else if (index == 2)
    //     m_provider = std::make_unique<AlphaVantageProvider>();
    // else if (index == 3)
    //     m_provider = std::make_unique<FinnhubProvider>();
    else
        m_provider = std::make_unique<StooqProvider>();

    qDebug() << "Provider switched to index:" << index;
}

void StockFetcher::start(const QStringList &symbols, int intervalMs)
{
    m_symbols = symbols;
    m_timer.start(intervalMs);

    fetch(m_symbols);
}

void StockFetcher::startBatched(const QStringList &allSymbols, int batchSize, int intervalMs)
{
    m_allSymbols = allSymbols;
    m_batchSize = batchSize;
    m_currentIndex = 0;

    m_timer.stop();
    disconnect(&m_timer, nullptr, this, nullptr);

    connect(&m_timer, &QTimer::timeout, this, &StockFetcher::fetchNextBatch);

    m_timer.start(intervalMs);
    fetchNextBatch();
}

void StockFetcher::fetchNextBatch()
{
    if (m_allSymbols.isEmpty() || m_batchSize <= 0)
        return;

    qsizetype end = std::min(
        static_cast<qsizetype>(m_currentIndex + m_batchSize),
        m_allSymbols.size()
        );

    QStringList batch = m_allSymbols.mid(m_currentIndex, end - m_currentIndex);

    qDebug() << "Fetching batch"
             << (m_currentIndex / m_batchSize + 1)
             << "of"
             << ((m_allSymbols.size() + m_batchSize - 1) / m_batchSize)
             << "(" << batch.size() << "stocks)";

    fetch(batch);

    m_currentIndex = end;

    if (m_currentIndex >= m_allSymbols.size())
        m_currentIndex = 0;
}

void StockFetcher::stop()
{
    m_timer.stop();
}

void StockFetcher::fetch(const QStringList &symbols)
{
    QNetworkReply *r = m_manager.get(QNetworkRequest(QUrl("https://www.google.com")));

    QObject::connect(r, &QNetworkReply::finished, []() {
        qDebug() << "Google request finished OK";
    });

    if (symbols.isEmpty())
        return;

    m_cache.clear();
    m_pendingReplies = symbols.size();

    for (const QString &symbol : symbols)
    {
        QString url = m_provider->buildUrl(symbol);

        QNetworkRequest request{QUrl(url)};
        request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

        QNetworkReply *reply = m_manager.get(request);

        connect(reply, &QNetworkReply::finished, this,
                [this, reply, symbol]()
                {
                    QByteArray data = reply->readAll();

                    int httpStatus =
                        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                            .toInt();

                    qDebug() << "Symbol:" << symbol << "HTTP:" << httpStatus;

                    if (reply->error() != QNetworkReply::NoError)
                    {
                        qWarning() << "Network error for" << symbol
                                   << ":" << reply->errorString();

                        reply->deleteLater();
                        finishPendingReply();
                        return;
                    }

                    Stock s = m_provider->parse(symbol, data);

                    if (s.price <= 0.0)
                        qWarning() << "Invalid stock data for" << symbol;

                    // update table model cache
                    m_cache.append(s);

                    reply->deleteLater();
                    finishPendingReply();
                });
    }
}

void StockFetcher::finishPendingReply()
{
    m_pendingReplies--;

    if (m_pendingReplies == 0)
    {
        std::sort(m_cache.begin(), m_cache.end(),
                  [](const Stock &a, const Stock &b)
                  {
                      return a.symbol < b.symbol;   // alphabetical
                  });

        m_model->setStocks(m_cache);
    }
}


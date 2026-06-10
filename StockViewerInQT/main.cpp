
// #include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <StockHistoryModel.h>
#include "stockhistorystore.h"

#include "StockModel.h"
#include "StockFetcher.h"
#include "alertmodel.h"
#include "logger.h"
#include "alpacawebsocket.h"
#include "alpacapositionprovider.h"
#include "positionmodel.h"
#include "finnhubwebsocket.h"
#include <QQmlContext>
#include <QTextStream>
#include <QFile>
#include <QQuickStyle>
#include <QTest>

// Reads KEY=VALUE pairs from a file, ignores blank lines and # comments.
// Call before anything that needs credentials.
static QMap<QString,QString> loadEnv(const QString &path)
{
    QMap<QString,QString> env;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[env] Could not open" << path
                   << "— set ALPACA_KEY and ALPACA_SECRET in .env";
        return env;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        const int eq = line.indexOf('=');
        if (eq < 1) continue;
        const QString key = line.left(eq).trimmed();
        const QString val = line.mid(eq + 1).trimmed();
        env[key] = val;
    }

    qDebug() << "[env] Loaded" << env.size() << "keys from" << path;
    return env;
}

static QList<QPair<QString,QString>> loadSymbolsFromCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open CSV:" << filePath << "— using fallback symbols";
        return {
                {"AAPL",  "Apple Inc."},
                {"MSFT",  "Microsoft Corporation"},
                {"NVDA",  "NVIDIA Corporation"},
                {"GOOGL", "Alphabet Inc."},
                {"TSLA",  "Tesla Inc."},
                {"AMZN",  "Amazon.com Inc."},
                {"META",  "Meta Platforms Inc."},
                {"JPM",   "JPMorgan Chase & Co."},
                };
    }

    QList<QPair<QString,QString>> result;
    QTextStream in(&file);
    in.readLine(); // skip header row

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        const QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;

        const QString symbol = parts[0].trimmed().toUpper();
        const QString name   = parts[1].trimmed();

        if (!symbol.isEmpty() && !symbol.contains('.'))
            result.append({ symbol, name });
    }

    qDebug() << "Loaded" << result.size() << "symbols from CSV";
    return result;
}

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Basic");
    qInstallMessageHandler(Logger::messageHandler);
    QApplication app(argc, argv);

    // Load Keys.env
    const QString envPath = QCoreApplication::applicationDirPath() + "/Keys.env";
    const QMap<QString,QString> env = loadEnv(envPath);

    const QString alpacaKey    = env.value("ALPACA_KEY");
    const QString alpacaSecret = env.value("ALPACA_SECRET");
    const QString finnhubKey   = env.value("FINNHUB_KEY");

    bool hasAlpaca = !alpacaKey.isEmpty() && !alpacaSecret.isEmpty();
    hasAlpaca = false;
    bool hasFinnhub = !finnhubKey.isEmpty();
    // hasFinnhub = false;
    qDebug() << "[main] Alpaca credentials:" << (hasAlpaca ? "found" : "NOT FOUND — live feed disabled");
    qDebug() << "[main] Finnhub API key:"    << (hasFinnhub ? "found" : "NOT FOUND — Finnhub provider disabled");

    // --- Models ---
    StockModel       browseModel;
    WatchlistModel   watchlist;
    StockHistoryStore historyStore;
    StockHistoryModel historyModel(&historyStore);
    AlertModel        alertModel;

    // --- Signal engine ---
    SignalEngine signalEngine(&historyStore, &watchlist, &alertModel, &app);

    // --- Fetcher ---
    StockFetcher fetcher(&watchlist, &historyStore, &app);

    // --- Trade tape ---
    TradeTickModel tradeTickModel(&watchlist, &app);

    // --- Alpaca WebSocket (live quotes) ---
    AlpacaWebSocket alpacaWs(&watchlist, &tradeTickModel, &app);
    // --- Finnhub WebSocket (live trades, free tier) ---
    FinnhubWebSocket finnhubWs(&watchlist, &tradeTickModel, &app);


    // --- Positions ---
    AlpacaPositionProvider positionProvider;
    PositionModel          positionModel;

    if (hasAlpaca) {
        alpacaWs.setCredentials(alpacaKey, alpacaSecret);
        fetcher.setAlpacaCredentials(alpacaKey, alpacaSecret);

        // Switch fetcher to Alpaca provider for history
        fetcher.setProvider(2);

        // Paper vs live - driven by ALPACA_MODE in Keys.env
        const bool paper = env.value("ALPACA_MODE", "paper") != "live";
        positionProvider.setCredentials(alpacaKey, alpacaSecret, paper);
        positionModel.setProvider(&positionProvider, 5000);
    }

    if (hasFinnhub) {
        fetcher.setFinnhubApiKey(finnhubKey);
        // Switch fetcher to Finnhub provider for history
        fetcher.setProvider(3);
        finnhubWs.setApiKey(finnhubKey);
    }

    // When history arrives for a symbol -> run signal analysis
    QObject::connect(&historyStore, &StockHistoryStore::historyUpdated,
                     &signalEngine, &SignalEngine::analyze);

    // When stock added to watchlist -> fetch price immediately
    QObject::connect(&watchlist, &WatchlistModel::countChanged,
                     &fetcher,   &StockFetcher::refreshNow);

    // --- Expose to QML ---
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("browseModel",   &browseModel);
    engine.rootContext()->setContextProperty("watchlistModel",     &watchlist);
    engine.rootContext()->setContextProperty("historyModel",  &historyModel);
    engine.rootContext()->setContextProperty("historyStore",  &historyStore);
    engine.rootContext()->setContextProperty("stockFetcher",  &fetcher);
    engine.rootContext()->setContextProperty("alertModel",    &alertModel);
    engine.rootContext()->setContextProperty("logger",       Logger::instance());
    engine.rootContext()->setContextProperty("alpacaWs",      &alpacaWs);
    engine.rootContext()->setContextProperty("hasAlpaca",     hasAlpaca);
    engine.rootContext()->setContextProperty("hasFinnhub",    hasFinnhub);
    engine.rootContext()->setContextProperty("finnhubWs",      &finnhubWs);
    engine.rootContext()->setContextProperty("positionModel", &positionModel);
    engine.rootContext()->setContextProperty("tradeTickModel", &tradeTickModel);

    // Load symbols
    auto symbols = loadSymbolsFromCSV(":/csv/sp500.csv");
    browseModel.setSymbols(symbols);

    engine.loadFromModule("StockViewerInQT", "Main");

    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QML failed to load";
        return -1;
    }

    if (hasAlpaca) {
        // Connect WebSocket — live prices come in via trades/quotes
        alpacaWs.connectToFeed();
        // Still poll REST every 60s as a safety net for missed WS ticks
        // fetcher.start(60000);
    } else if (hasFinnhub) {
        // Finnhub WS as primary live feed when Alpaca is unavailable
        finnhubWs.connectToFeed();
        // Finnhub WS is trades-only; still poll REST for history & prev-close
        fetcher.start(60'000);
    } else {
        // No live feed — fall back to Stooq/Yahoo REST polling every 30 s
        fetcher.start(30'000);
    }

#ifdef QT_DEBUG
    QTest::qExec(&fetcher);
#endif

    return app.exec();
}

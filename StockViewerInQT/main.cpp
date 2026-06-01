
// #include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <StockHistoryModel.h>
#include "stockhistorystore.h"

#include "StockModel.h"
#include "StockFetcher.h"
#include <QQmlContext>
#include <QTextStream>
#include <QFile>

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
    QApplication app(argc, argv);

    // --- Models ---
    StockModel       browseModel;
    WatchlistModel   watchlist;
    StockHistoryStore historyStore;
    StockHistoryModel historyModel(&historyStore);

    // --- Fetcher (only knows about the watchlist) ---
    StockFetcher fetcher(&watchlist, &historyStore, &app);

    // --- Expose to QML ---
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("browseModel",   &browseModel);
    engine.rootContext()->setContextProperty("watchlist",     &watchlist);
    engine.rootContext()->setContextProperty("historyModel",  &historyModel);
    engine.rootContext()->setContextProperty("historyStore",  &historyStore);
    engine.rootContext()->setContextProperty("stockFetcher",  &fetcher);

    engine.loadFromModule("StockViewerInQT", "Main");

    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QML failed to load";
        return -1;
    }

    // --- Load symbol list ---
    auto symbols = loadSymbolsFromCSV(":/csv/sp500.csv");
    browseModel.setSymbols(symbols);

    // --- Start price refresh timer (30s) ---
    fetcher.start(30000);

    return app.exec();
}

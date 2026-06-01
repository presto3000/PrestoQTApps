
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

QStringList loadSP500FromCSV(const QString &filePath)
{
    QStringList symbols;
    QFile file(filePath);
    return {"aapl", "msft", "nvda", "googl", "tsla"};
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open CSV:" << filePath;
        // Fallback symbols
        return {"aapl", "msft", "nvda", "googl", "tsla"};
    }

    QTextStream in(&file);
    QString line = in.readLine(); // skip header

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            QString symbol = parts.first().trimmed().toLower();
            if (!symbol.isEmpty() && !symbol.contains(".")) {  // clean symbol
                symbols << symbol;
            }
        }
    }

    qDebug() << "Loaded" << symbols.size() << "symbols from CSV";
    return symbols;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "App started";

    StockModel model;

    QQmlApplicationEngine engine;

    StockHistoryStore historyStore;
    StockHistoryModel historyModel(&historyStore);

    StockFetcher* fetcher = new StockFetcher(&model, &historyStore, &app);

    engine.rootContext()->setContextProperty("historyModel", &historyModel);
    engine.rootContext()->setContextProperty("historyStore", &historyStore);

    engine.rootContext()->setContextProperty("stockModel", &model);
    engine.rootContext()->setContextProperty("stockFetcher", fetcher);

    engine.loadFromModule("StockViewerInQT", "Main");

    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QML failed to load";
        return -1;
    }

    qDebug() << "QML loaded OK";

    QStringList allSymbols = loadSP500FromCSV(":/csv/sp500.csv");

    fetcher->startBatched(allSymbols, 100, 5000);

    return app.exec();
}

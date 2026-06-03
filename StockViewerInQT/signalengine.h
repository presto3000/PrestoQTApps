#ifndef SIGNALENGINE_H
#define SIGNALENGINE_H

#include <QObject>
#include "stockhistorystore.h"
#include "watchlistmodel.h"

struct TechSignal {
    QString symbol;
    QString name;           // e.g. "RSI Oversold", "Golden Cross"
    QString description;    // detail text shown in popup
    QString type;           // "bullish" | "bearish" | "neutral"
    double  value;          // the indicator value that triggered
    QDateTime detectedAt;
};

class AlertModel;

class SignalEngine : public QObject
{
    Q_OBJECT
public:
    explicit SignalEngine(StockHistoryStore *store,
                          WatchlistModel   *watchlist,
                          AlertModel       *alerts,
                          QObject          *parent = nullptr);

    // Call this after new history arrives for a symbol
    Q_INVOKABLE void analyze(const QString &symbol);

private:
    // Indicators - all return NaN if not enough data
    /*
    Calculates the Relative Strength Index (RSI) for the given price series over the specified period.
    RSI measures the magnitude of recent price changes to evaluate overbought or oversold conditions.
    Returns the RSI value in the range 0–100.
    Requires at least period + 1 data points.
    */
    double calcRSI(const QVector<PricePoint> &pts, int period = 14) const;
    /*
    Computes the Simple Moving Average (SMA) of closing prices over the given period.
    SMA smooths short‑term fluctuations and highlights the underlying trend.
    Returns the arithmetic mean of the last period closing prices.
    */
    double calcSMA(const QVector<PricePoint> &pts, int period) const;
    /*
    Calculates the momentum indicator, defined as the difference between the current closing price and the closing price period bars earlier.
    Momentum helps identify the speed and direction of price movement.
    Returns a positive value for upward momentum and negative for downward momentum.
    */
    double calcMomentum(const QVector<PricePoint> &pts, int period = 10) const;

    StockHistoryStore *m_store;
    WatchlistModel    *m_watchlist;
    AlertModel        *m_alerts;
};

#endif // SIGNALENGINE_H

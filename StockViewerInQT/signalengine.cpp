#include "signalengine.h"

#include "alertmodel.h"
#include <QtMath>
#include <QDebug>

SignalEngine::SignalEngine(StockHistoryStore *store,
                           WatchlistModel   *watchlist,
                           AlertModel       *alerts,
                           QObject          *parent)
    : QObject(parent),
    m_store(store),
    m_watchlist(watchlist),
    m_alerts(alerts)
{}

// === Indicators ===

double SignalEngine::calcRSI(const QVector<PricePoint> &pts, int period) const
{
    if (pts.size() < period + 1)
        return qQNaN();

    double avgGain = 0.0, avgLoss = 0.0;

    for (int i = pts.size() - period; i < pts.size(); ++i) {
        double diff = pts[i].price - pts[i - 1].price;
        if (diff > 0) avgGain += diff;
        else          avgLoss -= diff;
    }

    avgGain /= period;
    avgLoss /= period;

    if (avgLoss == 0.0) return 100.0;

    double rs = avgGain / avgLoss;
    return 100.0 - (100.0 / (1.0 + rs));
}

double SignalEngine::calcSMA(const QVector<PricePoint> &pts, int period) const
{
    if (pts.size() < period)
        return qQNaN();

    double sum = 0.0;
    for (int i = pts.size() - period; i < pts.size(); ++i)
        sum += pts[i].price;

    return sum / period;
}

double SignalEngine::calcMomentum(const QVector<PricePoint> &pts, int period) const
{
    if (pts.size() < period + 1)
        return qQNaN();

    double current = pts.last().price;
    double past    = pts[pts.size() - 1 - period].price;

    if (past == 0.0) return qQNaN();
    return ((current - past) / past) * 100.0;   // % change over period
}

// === Analysis ===

void SignalEngine::analyze(const QString &symbol)
{
    const QVector<PricePoint> pts = m_store->history(symbol);

    if (pts.size() < 51)   // need at least 50 bars for SMA50
        return;

    const double rsi      = calcRSI(pts, 14);
    const double sma20    = calcSMA(pts, 20);
    const double sma50    = calcSMA(pts, 50);
    const double momentum = calcMomentum(pts, 10);
    const double price    = pts.last().price;

    // Previous SMAs (one bar ago) for crossover detection
    QVector<PricePoint> prev = pts;
    prev.removeLast();
    const double prevSma20 = calcSMA(prev, 20);
    const double prevSma50 = calcSMA(prev, 50);

    qDebug() << "[SignalEngine]" << symbol
             << "RSI:" << rsi
             << "SMA20:" << sma20
             << "SMA50:" << sma50
             << "Mom:" << momentum;

    // === Signal 1: RSI Oversold ===
    if (!qIsNaN(rsi) && rsi < 30.0) {
        TechSignal sig;
        sig.symbol      = symbol;
        sig.name        = "RSI Oversold";
        sig.type        = "bullish";
        sig.value       = rsi;
        sig.detectedAt  = QDateTime::currentDateTime();
        sig.description = QString::asprintf(
            "%s RSI is %.1f — below 30, indicating oversold conditions.\n\n"
            "Current price: $%.2f\n"
            "RSI(14): %.1f\n\n"
            "Historically, RSI below 30 suggests the stock may be due for a "
            "mean-reversion bounce. Not a guarantee — confirm with volume and trend.",
            qPrintable(symbol), rsi, price, rsi);
        m_alerts->addSignal(sig);
    }

    // === Signal 2: RSI Overbought ===
    if (!qIsNaN(rsi) && rsi > 70.0) {
        TechSignal sig;
        sig.symbol      = symbol;
        sig.name        = "RSI Overbought";
        sig.type        = "bearish";
        sig.value       = rsi;
        sig.detectedAt  = QDateTime::currentDateTime();
        sig.description = QString::asprintf(
            "%s RSI is %.1f — above 70, indicating overbought conditions.\n\n"
            "Current price: $%.2f\n"
            "RSI(14): %.1f\n\n"
            "RSI above 70 suggests the stock may be overextended. "
            "Watch for momentum fading or a pullback to the 20-day SMA ($%.2f).",
            qPrintable(symbol), rsi, price, rsi, sma20);
        m_alerts->addSignal(sig);
    }

    // === Signal 3: Golden Cross ===
    if (!qIsNaN(sma20) && !qIsNaN(sma50) &&
        !qIsNaN(prevSma20) && !qIsNaN(prevSma50) &&
        prevSma20 <= prevSma50 && sma20 > sma50)
    {
        TechSignal sig;
        sig.symbol      = symbol;
        sig.name        = "Golden Cross";
        sig.type        = "bullish";
        sig.value       = sma20;
        sig.detectedAt  = QDateTime::currentDateTime();
        sig.description = QString::asprintf(
            "%s just formed a Golden Cross.\n\n"
            "Current price: $%.2f\n"
            "SMA(20): $%.2f  →  crossed above  ←  SMA(50): $%.2f\n\n"
            "The 20-day moving average has crossed above the 50-day, a classic "
            "bullish momentum signal. Often followed by a sustained uptrend, "
            "though false signals occur in choppy markets.",
            qPrintable(symbol), price, sma20, sma50);
        m_alerts->addSignal(sig);
    }

    // === Signal 4: Death Cross ===
    if (!qIsNaN(sma20) && !qIsNaN(sma50) &&
        !qIsNaN(prevSma20) && !qIsNaN(prevSma50) &&
        prevSma20 >= prevSma50 && sma20 < sma50)
    {
        TechSignal sig;
        sig.symbol      = symbol;
        sig.name        = "Death Cross";
        sig.type        = "bearish";
        sig.value       = sma20;
        sig.detectedAt  = QDateTime::currentDateTime();
        sig.description = QString::asprintf(
            "%s just formed a Death Cross.\n\n"
            "Current price: $%.2f\n"
            "SMA(20): $%.2f  →  crossed below  ←  SMA(50): $%.2f\n\n"
            "The 20-day moving average has dropped below the 50-day — a bearish "
            "signal that often precedes further downside. Consider tightening "
            "stop-losses or reducing exposure.",
            qPrintable(symbol), price, sma20, sma50);
        m_alerts->addSignal(sig);
    }

    // === Signal 5: Strong Momentum ===
    if (!qIsNaN(momentum) && momentum > 10.0) {
        TechSignal sig;
        sig.symbol      = symbol;
        sig.name        = "Strong Momentum";
        sig.type        = "bullish";
        sig.value       = momentum;
        sig.detectedAt  = QDateTime::currentDateTime();
        sig.description = QString::asprintf(
            "%s has surged %.1f%% over the last 10 sessions.\n\n"
            "Current price: $%.2f\n"
            "10-day momentum: +%.1f%%\n"
            "RSI(14): %.1f\n\n"
            "Strong positive momentum can continue (trend-following) or revert "
            "(mean-reversion). RSI context matters: if RSI is also above 70, "
            "the move may be overextended.",
            qPrintable(symbol), momentum, price, momentum, rsi);
        m_alerts->addSignal(sig);
    }

    // === Signal 6: Extended Below SMA50 ===
    if (!qIsNaN(sma50) && sma50 > 0.0) {
        double pctFromSma50 = ((price - sma50) / sma50) * 100.0;
        if (pctFromSma50 < -10.0) {
            TechSignal sig;
            sig.symbol      = symbol;
            sig.name        = "Extended Below SMA50";
            sig.type        = "neutral";
            sig.value       = pctFromSma50;
            sig.detectedAt  = QDateTime::currentDateTime();
            sig.description = QString::asprintf(
                "%s is trading %.1f%% below its 50-day moving average.\n\n"
                "Current price: $%.2f\n"
                "SMA(50): $%.2f\n"
                "Distance: %.1f%%\n\n"
                "A large gap below SMA50 can indicate either a value opportunity "
                "or a stock in a downtrend. Check earnings, news, and sector context "
                "before acting. RSI(14) is currently %.1f.",
                qPrintable(symbol), -pctFromSma50, price, sma50, pctFromSma50, rsi);
            m_alerts->addSignal(sig);
        }
    }
}
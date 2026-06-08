#ifndef IPOSITIONPROVIDER_H
#define IPOSITIONPROVIDER_H

#include <QObject>
#include <QString>
#include <QList>

// -- Data ----------------------------------------------------------------------

struct Position {
    QString symbol;
    QString name;
    double  qty          = 0.0;   // shares held (negative = short)
    double  avgEntryPrice = 0.0;
    double  currentPrice  = 0.0;
    double  marketValue   = 0.0;
    double  unrealizedPL  = 0.0;
    double  unrealizedPLPct = 0.0;
    double  costBasis     = 0.0;
    QString side;                 // "long" | "short"
};

// -- Interface -----------------------------------------------------------------

class IPositionProvider : public QObject
{
    Q_OBJECT

public:
    explicit IPositionProvider(QObject *parent = nullptr)
        : QObject(parent) {}

    virtual ~IPositionProvider() = default;

    virtual QString name()    const = 0;
    virtual bool    isPaper() const = 0;

    // Trigger a fetch - results come back via positionsFetched signal
    virtual void fetchPositions() = 0;

    // Close a single position by symbol (market order)
    virtual void closePosition(const QString &symbol) = 0;

signals:
    void positionsFetched(const QList<Position> &positions);
    void positionClosed(const QString &symbol, bool success, const QString &error);
    void fetchError(const QString &error);
};

#endif // IPOSITIONPROVIDER_H

#ifndef POSITIONMODEL_H
#define POSITIONMODEL_H

#include <QAbstractListModel>
#include <QTimer>
#include "ipositionprovider.h"

class PositionModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int     count          READ rowCount       NOTIFY countChanged)
    Q_PROPERTY(double  totalPL        READ totalPL        NOTIFY dataUpdated)
    Q_PROPERTY(double  totalValue     READ totalValue     NOTIFY dataUpdated)
    Q_PROPERTY(bool    loading        READ isLoading      NOTIFY loadingChanged)
    Q_PROPERTY(QString providerName   READ providerName   NOTIFY providerChanged)
    Q_PROPERTY(bool    isPaper        READ isPaper        NOTIFY providerChanged)

public:
    enum Roles {
        SymbolRole = Qt::UserRole + 1,
        NameRole,
        QtyRole,
        AvgEntryRole,
        CurrentPriceRole,
        MarketValueRole,
        UnrealizedPLRole,
        UnrealizedPLPctRole,
        CostBasisRole,
        SideRole
    };

    explicit PositionModel(QObject *parent = nullptr);

    // Swap provider at runtime - paper -> live or different broker
    void setProvider(IPositionProvider *provider, int refreshIntervalMs = 5000);

    int     rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    double  totalPL()     const;
    double  totalValue()  const;
    bool    isLoading()   const { return m_loading; }
    QString providerName()const;
    bool    isPaper()     const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void closePosition(const QString &symbol);

signals:
    void countChanged();
    void dataUpdated();
    void loadingChanged();
    void providerChanged();
    void closeError(const QString &symbol, const QString &error);

private:
    void onPositionsFetched(const QList<Position> &positions);
    void onPositionClosed(const QString &symbol, bool ok, const QString &err);

    IPositionProvider *m_provider = nullptr;
    QList<Position>    m_positions;
    QTimer             m_timer;
    bool               m_loading = false;
};

#endif // POSITIONMODEL_H

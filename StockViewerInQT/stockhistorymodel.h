#ifndef STOCKHISTORYMODEL_H
#define STOCKHISTORYMODEL_H

#include <QAbstractListModel>
#include "stockhistorystore.h"

class StockHistoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString symbol READ symbol WRITE setSymbol NOTIFY symbolChanged)

public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        PriceRole
    };

    explicit StockHistoryModel(StockHistoryStore *store, QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString symbol() const;
    Q_INVOKABLE void setSymbol(const QString &symbol);
    Q_INVOKABLE double priceAt(int index) const;

signals:
    void symbolChanged();

private slots:
    void onHistoryUpdated(const QString &symbol);

private:
    QString m_symbol;
    QVector<PricePoint> m_points;
    StockHistoryStore *m_store;
};

#endif
#ifndef WATCHLISTMODEL_H
#define WATCHLISTMODEL_H

#include <QAbstractListModel>
#include "stockmodel.h"

class WatchlistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        SymbolRole = Qt::UserRole + 1,
        NameRole,
        PriceRole,
        ChangeRole,      // absolute change from previous close
        ChangePctRole    // % change
    };

    explicit WatchlistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool addStock(const QString &symbol, const QString &name);
    Q_INVOKABLE void removeStock(const QString &symbol);
    Q_INVOKABLE bool contains(const QString &symbol) const;
    Q_INVOKABLE QStringList symbols() const;

    // Called by StockFetcher when a price arrives
    void updatePrice(const QString &symbol, double price, double prevClose);

signals:
    void countChanged();

private:
    struct WatchEntry {
        QString symbol;
        QString name;
        double price = 0.0;
        double prevClose = 0.0;
    };

    QList<WatchEntry> m_entries;
};

#endif // WATCHLISTMODEL_H

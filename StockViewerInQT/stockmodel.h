#ifndef STOCKMODEL_H
#define STOCKMODEL_H

#include <QAbstractTableModel>

struct Stock {
    QString symbol;
    QString name;
    double price;
};

class StockModel : public QAbstractListModel {
    Q_OBJECT

    public:
        enum Roles {
            SymbolRole = Qt::UserRole + 1,
            NameRole,
            PriceRole
        };

        explicit StockModel(QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

        void setStocks(const QList<Stock> &stocks);

    private:
        QList<Stock> m_stocks;
};

#endif // STOCKMODEL_H

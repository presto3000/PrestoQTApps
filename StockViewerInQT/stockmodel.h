#ifndef STOCKMODEL_H
#define STOCKMODEL_H

#include <QAbstractTableModel>

struct Stock {
    QString symbol;
    QString name;
    double  price     = 0.0;
    double  prevClose = 0.0;   // used for % change calculation
};

class StockModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    enum Roles {
        SymbolRole = Qt::UserRole + 1,
        NameRole
    };

    explicit StockModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Load the full symbol list (called once at startup from CSV)
    void setSymbols(const QList<QPair<QString, QString>> &symbolsAndNames);

    QString filter() const;
    void setFilter(const QString &text);

signals:
    void filterChanged();

private:
    struct Entry { QString symbol; QString name; };

    QList<Entry> m_all;       // full list
    QList<Entry> m_filtered;  // what the view sees
    QString      m_filter;

    void applyFilter();
};

#endif // STOCKMODEL_H

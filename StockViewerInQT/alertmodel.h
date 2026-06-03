#ifndef ALERTMODEL_H
#define ALERTMODEL_H

#include <QAbstractListModel>
#include "signalengine.h"

class AlertModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        SymbolRole = Qt::UserRole + 1,
        SignalNameRole,
        DescriptionRole,
        TypeRole,
        ValueRole,
        TimeRole
    };

    explicit AlertModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addSignal(const TechSignal &sig);
    Q_INVOKABLE void clear()
    {
        beginResetModel();
        m_alerts.clear();
        endResetModel();
        emit countChanged();
    }

    // Fire a fake signal for testing — cycles through all types
    Q_INVOKABLE void addTestSignal(const QString &symbol = "TEST");

signals:
    void countChanged();
    void newSignal(const QString &symbol, const QString &name, const QString &type);

private:
    QList<TechSignal> m_alerts;
    static constexpr int MAX_ALERTS = 100;
};

#endif // ALERTMODEL_H

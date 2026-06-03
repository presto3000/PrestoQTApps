#include "alertmodel.h"

AlertModel::AlertModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int AlertModel::rowCount(const QModelIndex &) const
{
    return m_alerts.size();
}

QVariant AlertModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_alerts.size())
        return {};

    const TechSignal &s = m_alerts[index.row()];

    switch (role) {
    case SymbolRole:      return s.symbol;
    case SignalNameRole:  return s.name;
    case DescriptionRole: return s.description;
    case TypeRole:        return s.type;
    case ValueRole:       return s.value;
    case TimeRole:        return s.detectedAt.toString("hh:mm  dd MMM");
    }
    return {};
}

QHash<int, QByteArray> AlertModel::roleNames() const
{
    return {
        { SymbolRole,      "symbol"      },
        { SignalNameRole,  "signalName"  },
        { DescriptionRole, "description" },
        { TypeRole,        "signalType"  },
        { ValueRole,       "signalValue" },
        { TimeRole,        "signalTime"  }
    };
}

void AlertModel::addSignal(const TechSignal &sig)
{
    // Deduplicate: drop if same symbol+signal already at top
    if (!m_alerts.isEmpty()) {
        const auto &top = m_alerts.first();
        if (top.symbol == sig.symbol && top.name == sig.name)
            return;
    }

    beginInsertRows({}, 0, 0);
    m_alerts.prepend(sig);
    endInsertRows();

    if (m_alerts.size() > MAX_ALERTS) {
        beginRemoveRows({}, m_alerts.size() - 1, m_alerts.size() - 1);
        m_alerts.removeLast();
        endRemoveRows();
    }

    emit countChanged();
    emit newSignal(sig.symbol, sig.name, sig.type);
}

void AlertModel::addTestSignal(const QString &symbol)
{
    static const QList<QPair<QString,QString>> types = {
                                                         { "RSI Oversold",        "bullish" },
                                                         { "RSI Overbought",      "bearish" },
                                                         { "Golden Cross",        "bullish" },
                                                         { "Death Cross",         "bearish" },
                                                         { "Strong Momentum",     "bullish" },
                                                         { "Extended Below SMA50","neutral" },
                                                         };

    static int counter = 0;
    const auto &[name, type] = types[counter % types.size()];
    counter++;

    TechSignal sig;
    sig.symbol      = symbol.toUpper();
    sig.name        = name;
    sig.type        = type;
    sig.value       = 25.0 + (counter * 7.3);
    sig.detectedAt  = QDateTime::currentDateTime();
    sig.description = QString(
                          "This is a TEST signal for %1.\n\n"
                          "Signal: %2\nType: %3\nValue: %.2f\n\n"
                          "This was triggered manually from the debug panel "
                          "to verify that the alert table and detail popup are working correctly."
                          ).arg(sig.symbol, sig.name, sig.type).arg(sig.value);

    addSignal(sig);
}
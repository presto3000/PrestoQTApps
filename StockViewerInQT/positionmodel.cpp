#include "positionmodel.h"

#include <QDebug>

PositionModel::PositionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &PositionModel::refresh);
}

// -- Provider swap -------------------------------------------------------------

void PositionModel::setProvider(IPositionProvider *provider, int intervalMs)
{
    // Disconnect old provider if any
    if (m_provider)
        m_provider->disconnect(this);

    m_provider = provider;
    m_timer.stop();

    if (!m_provider)
        return;

    connect(m_provider, &IPositionProvider::positionsFetched,
            this,        &PositionModel::onPositionsFetched);

    connect(m_provider, &IPositionProvider::positionClosed,
            this,        &PositionModel::onPositionClosed);

    connect(m_provider, &IPositionProvider::fetchError,
            this, [this](const QString &err) {
                qWarning() << "[PositionModel] provider error:" << err;
                m_loading = false;
                emit loadingChanged();
            });

    emit providerChanged();

    m_timer.start(intervalMs);
    refresh();
}

// -- QAbstractListModel --------------------------------------------------------

int PositionModel::rowCount(const QModelIndex &) const
{
    return m_positions.size();
}

QVariant PositionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_positions.size())
        return {};

    const Position &p = m_positions[index.row()];

    switch (role) {
    case SymbolRole:         return p.symbol;
    case NameRole:           return p.name;
    case QtyRole:            return p.qty;
    case AvgEntryRole:       return p.avgEntryPrice;
    case CurrentPriceRole:   return p.currentPrice;
    case MarketValueRole:    return p.marketValue;
    case UnrealizedPLRole:   return p.unrealizedPL;
    case UnrealizedPLPctRole:return p.unrealizedPLPct;
    case CostBasisRole:      return p.costBasis;
    case SideRole:           return p.side;
    }
    return {};
}

QHash<int, QByteArray> PositionModel::roleNames() const
{
    return {
            { SymbolRole,          "symbol"          },
            { NameRole,            "name"            },
            { QtyRole,             "qty"             },
            { AvgEntryRole,        "avgEntry"        },
            { CurrentPriceRole,    "currentPrice"    },
            { MarketValueRole,     "marketValue"     },
            { UnrealizedPLRole,    "unrealizedPL"    },
            { UnrealizedPLPctRole, "unrealizedPLPct" },
            { CostBasisRole,       "costBasis"       },
            { SideRole,            "side"            },
            };
}

// -- Aggregates ----------------------------------------------------------------

double PositionModel::totalPL() const
{
    double sum = 0.0;
    for (const auto &p : m_positions)
        sum += p.unrealizedPL;
    return sum;
}

double PositionModel::totalValue() const
{
    double sum = 0.0;
    for (const auto &p : m_positions)
        sum += p.marketValue;
    return sum;
}

QString PositionModel::providerName() const
{
    return m_provider ? m_provider->name() : "No provider";
}

bool PositionModel::isPaper() const
{
    return m_provider ? m_provider->isPaper() : true;
}

// -- Invokables ----------------------------------------------------------------

void PositionModel::refresh()
{
    if (!m_provider) return;

    m_loading = true;
    emit loadingChanged();

    m_provider->fetchPositions();
}

void PositionModel::closePosition(const QString &symbol)
{
    if (!m_provider) return;
    qDebug() << "[PositionModel] closePosition:" << symbol;
    m_provider->closePosition(symbol);
}

// -- Private slots -------------------------------------------------------------

void PositionModel::onPositionsFetched(const QList<Position> &positions)
{
    beginResetModel();
    m_positions = positions;
    endResetModel();

    m_loading = false;
    emit loadingChanged();
    emit countChanged();
    emit dataUpdated();
}

void PositionModel::onPositionClosed(const QString &symbol,
                                     bool           ok,
                                     const QString &err)
{
    if (!ok) {
        qWarning() << "[PositionModel] close failed for" << symbol << ":" << err;
        emit closeError(symbol, err);
    } else {
        qDebug() << "[PositionModel] closed" << symbol << "successfully";
    }
}
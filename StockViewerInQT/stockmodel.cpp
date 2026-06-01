#include "stockmodel.h"

StockModel::StockModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int StockModel::rowCount(const QModelIndex &) const
{
    return m_filtered.size();
}

QVariant StockModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filtered.size())
        return {};

    const Entry &e = m_filtered[index.row()];

    switch (role) {
    case SymbolRole: return e.symbol;
    case NameRole:   return e.name;
    }
    return {};
}

QHash<int, QByteArray> StockModel::roleNames() const
{
    return {
        { SymbolRole, "symbol" },
        { NameRole,   "name"   }
    };
}

void StockModel::setSymbols(const QList<QPair<QString, QString>> &symbolsAndNames)
{
    m_all.clear();
    for (const auto &pair : symbolsAndNames)
        m_all.append({ pair.first.toUpper(), pair.second });

    applyFilter();
}

QString StockModel::filter() const
{
    return m_filter;
}

void StockModel::setFilter(const QString &text)
{
    if (m_filter == text)
        return;
    m_filter = text;
    applyFilter();
    emit filterChanged();
}

void StockModel::applyFilter()
{
    beginResetModel();
    if (m_filter.trimmed().isEmpty()) {
        m_filtered = m_all;
    } else {
        m_filtered.clear();
        const QString lower = m_filter.toLower();
        for (const auto &e : m_all) {
            if (e.symbol.toLower().contains(lower) ||
                e.name.toLower().contains(lower))
            {
                m_filtered.append(e);
            }
        }
    }
    endResetModel();
}
#include "stockmodel.h"

StockModel::StockModel(QObject *parent)
    : QAbstractListModel(parent) {}

int StockModel::rowCount(const QModelIndex &) const {
    return m_stocks.size();
}

int StockModel::columnCount(const QModelIndex &) const {
    return 3;
}

QVariant StockModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};

    const Stock &s = m_stocks[index.row()];

    switch (role) {
    case SymbolRole: return s.symbol;
    case NameRole: return s.name;
    case PriceRole: return s.price;
    }

    return {};
}

QHash<int, QByteArray> StockModel::roleNames() const {

    qDebug() << "roleNames:";
    return {
        {SymbolRole, "symbol"},
        {NameRole, "name"},
        {PriceRole, "price"}
    };
}

void StockModel::setStocks(const QList<Stock> &stocks) {
    beginResetModel();
    m_stocks = stocks;
    endResetModel();
}
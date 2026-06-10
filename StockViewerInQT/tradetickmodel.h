#ifndef TRADETICKMODEL_H
#define TRADETICKMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

class WatchlistModel;

struct TradeTick {
    QDateTime time;
    QString   symbol;
    double    price    = 0.0;
    int       size     = 0;
    QString   side;       // "buy" | "sell" | "" (unknown)
    QString   exchange;   // e.g. "EDGX", "ARCA", "NYSE", "BATS", "NASDAQ"
};

class TradeTickModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString symbol    READ symbol    WRITE setSymbol NOTIFY symbolChanged)
    Q_PROPERTY(int     count     READ rowCount  NOTIFY countChanged)
    Q_PROPERTY(double  bid       READ bid       NOTIFY spreadChanged)
    Q_PROPERTY(double  ask       READ ask       NOTIFY spreadChanged)
    Q_PROPERTY(int     bidSize   READ bidSize   NOTIFY spreadChanged)
    Q_PROPERTY(int     askSize   READ askSize   NOTIFY spreadChanged)
    Q_PROPERTY(double  spread    READ spread    NOTIFY spreadChanged)
    Q_PROPERTY(double  spreadPct READ spreadPct NOTIFY spreadChanged)

public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        PriceRole,
        SizeRole,
        SideRole,
        SymbolRole,
        ExchangeRole
    };

    explicit TradeTickModel(WatchlistModel *watchlist, QObject *parent = nullptr);

    int      rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString symbol()    const { return m_symbol; }
    void    setSymbol(const QString &s);

    double bid()       const { return m_bid; }
    double ask()       const { return m_ask; }
    int    bidSize()   const { return m_bidSize; }
    int    askSize()   const { return m_askSize; }
    double spread()    const { return m_ask > 0 && m_bid > 0 ? m_ask - m_bid : 0.0; }
    double spreadPct() const {
        return m_ask > 0 && m_bid > 0
                   ? ((m_ask - m_bid) / m_bid) * 100.0
                   : 0.0;
    }

    // Called by AlpacaWebSocket
    void addTick(const TradeTick &tick);
    void updateSpread(const QString &symbol,
                      double bid, int bidSize,
                      double ask, int askSize);

signals:
    void symbolChanged();
    void countChanged();
    void spreadChanged();

private:
    void seedSpreadFromWatchlist();   // seeds bid/ask on symbol switch

    WatchlistModel  *m_watchlist;
    QString          m_symbol;
    QList<TradeTick> m_ticks;

    double m_bid     = 0.0;
    double m_ask     = 0.0;
    int    m_bidSize = 0;
    int    m_askSize = 0;

    static constexpr int MAX_TICKS = 200;
};


#endif // TRADETICKMODEL_H

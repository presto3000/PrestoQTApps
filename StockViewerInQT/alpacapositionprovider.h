#ifndef ALPACAPOSITIONPROVIDER_H
#define ALPACAPOSITIONPROVIDER_H

#include "ipositionprovider.h"
#include <QNetworkAccessManager>

class AlpacaPositionProvider : public IPositionProvider
{
    Q_OBJECT

public:
    explicit AlpacaPositionProvider(QObject *parent = nullptr);

    void setCredentials(const QString &key,
                        const QString &secret,
                        bool           paper = true);

    QString name()    const override { return m_paper ? "Alpaca Paper" : "Alpaca Live"; }
    bool    isPaper() const override { return m_paper; }

    void fetchPositions()               override;
    void closePosition(const QString &symbol) override;

private:
    QNetworkRequest makeRequest(const QString &path) const;
    QString baseUrl() const;

    QNetworkAccessManager m_manager;
    QString m_key;
    QString m_secret;
    bool    m_paper = true;
};

#endif // ALPACAPOSITIONPROVIDER_H

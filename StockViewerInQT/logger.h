#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QStringList>
#include <QDateTime>

class Logger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList entries READ entries NOTIFY entriesChanged)

public:
    static Logger *instance();

    // Called by Qt's message handler — captures all qDebug/qWarning output
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &ctx,
                               const QString &msg);

    Q_INVOKABLE void log(const QString &msg);
    Q_INVOKABLE void clear();

    QStringList entries() const { return m_entries; }

signals:
    void entriesChanged();

private:
    explicit Logger(QObject *parent = nullptr);

    void append(const QString &line);

    QStringList m_entries;
    static constexpr int MAX_ENTRIES = 300;

    static Logger *s_instance;
};

#endif // LOGGER_H

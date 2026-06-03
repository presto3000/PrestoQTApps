#include "logger.h"

#include <QDebug>

Logger *Logger::s_instance = nullptr;

Logger::Logger(QObject *parent)
    : QObject(parent)
{}

Logger *Logger::instance()
{
    if (!s_instance)
        s_instance = new Logger();
    return s_instance;
}

void Logger::messageHandler(QtMsgType type,
                            const QMessageLogContext &,
                            const QString &msg)
{
    QString prefix;
    switch (type) {
    case QtDebugMsg:    prefix = "  ";        break;
    case QtInfoMsg:     prefix = "  INFO  ";  break;
    case QtWarningMsg:  prefix = "  WARN  ";  break;
    case QtCriticalMsg: prefix = "  CRIT  ";  break;
    case QtFatalMsg:    prefix = "  FATAL ";  break;
    }

    const QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    const QString line = time + prefix + msg;

    // Also print to stderr so Qt Creator output still works
    fprintf(stderr, "%s\n", qPrintable(line));

    if (s_instance)
        s_instance->append(line);
}

void Logger::log(const QString &msg)
{
    const QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    append(time + "  QML  " + msg);
}

void Logger::clear()
{
    m_entries.clear();
    emit entriesChanged();
}

void Logger::append(const QString &line)
{
    m_entries.append(line);   // newest first

    if (m_entries.size() > MAX_ENTRIES)
        m_entries.removeLast();

    emit entriesChanged();
}
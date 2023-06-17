#include "uciengine.h"
#include <QDebug>
#include <QObject>

UciEngine::UciEngine(QObject *parent)
    : QObject{parent}
{
    m_uciEngine = new QProcess(this);
    m_uciEngine->setReadChannel(QProcess::StandardOutput);

    connect(m_uciEngine, SIGNAL(readyRead()), SLOT(readFromEngine()));
}

// TODO: Add stopEngine that calls QProcess.close() method.
void UciEngine::startEngine(const QString &enginepath)
{
    qInfo() << Q_FUNC_INFO;
    m_uciEngine->start(enginepath);
}

void UciEngine::stopEngine()
{
    qInfo() << Q_FUNC_INFO;
    m_uciEngine->close();
}

void UciEngine::sendCommand(const QString &command)
{
    qInfo() << Q_FUNC_INFO;
    qInfo() << command.toLatin1();
    m_uciEngine->write(command.toLatin1() + "\n");
}

void UciEngine::readFromEngine()
{
    while (m_uciEngine->canReadLine()){
        QByteArray line = m_uciEngine->readLine().trimmed();
        parseLine(line);
        emit messageReceived(line);
    }
}

void UciEngine::parseLine(const QString& line)
{
    if (line.contains("bestmove"))
    {
        qInfo() << line;
        QStringList bestMove = line.split(" ");
        qInfo() << bestMove[1];

        emit engineMove(QString(bestMove[1]));
    }
}

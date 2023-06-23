#ifndef UCIENGINE_H
#define UCIENGINE_H

#include <QObject>
#include <QProcess>

/**
 * @brief The UciEngine class
 * The code in this class is partly based on https://github.com/cutechess/cutechess.
 */

class UciEngine : public QObject
{
    Q_OBJECT

public:
    explicit UciEngine(QObject *parent = nullptr);
    ~UciEngine();

public slots:
    void startEngine(const QString &enginepath);
    void stopEngine();
    void sendCommand(const QString &command);

private slots:
    void readFromEngine();

signals:
    void messageReceived(QString line);
    void engineMove(QString);

private:
    void parseLine(const QString &line);
    QProcess *m_uciEngine;
};

#endif // UCIENGINE_H

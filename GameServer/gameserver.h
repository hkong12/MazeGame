#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QStringList>
#include <QTcpServer>
#include <QMutex>
#include <QTimer>
#include <QList>

class GameState;

class GameServer : public QTcpServer
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};
    GameServer(QObject *parent = 0);

    QString newClient();
    bool newMove(QString &playerID, const QString &move);
    GameState* getGameState() { return m_gameState; }

signals:
    void gameStart();

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

private slots:
    void handleWaitingTimeout();

private:
    void getRandString(QString &str);

    Status m_serverStatus;
    QTimer *m_timer;
    QMutex m_gameStateMutex;
    QMutex m_serverStatusMutex;
    QList<QString> m_playerList;
    GameState* m_gameState;
};

#endif // GAMESERVER_H

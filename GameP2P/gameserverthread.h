#ifndef GAMESERVERTHREAD_H
#define GAMESERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include "gameserver.h"
#include "connection.h"

class GameServerThread : QObject
{
    Q_OBJECT

public:
    GameServerThread(GameServer* server, int socketDiscriptor, QObject *parent);
    ~GameServerThread();

signals:
    void error(QTcpSocket::SocketError);

private slots:
    void handleNewClient();
    void handleNewMove(QString &playerID, const QString &move);
    void handleDoneTcpSocket();
    void handleGameStart();

private:
    int m_socketDiscriptor;
    QThread m_thread;
    GameServer* m_serverPtr;
    Connection* m_connection;
};

#endif // GAMESERVERTHREAD_H

#ifndef GAMESERVERTHREAD_H
#define GAMESERVERTHREAD_H

#include <QThread>
#include "gameserver.h"

class GameServerThread : public QThread
{
    Q_OBJECT

public:
    GameServerThread(GameServer* server, Connection* conn, QThread *parent = 0);

public slots:
    void handleNewMove(QByteArray bytes);
    void handleGameStart();

private:
    GameServer* m_server;
    Connection* m_conn;
};

#endif // GAMESERVERTHREAD_H

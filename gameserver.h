#ifndef GAMESERVER_H
#define GAMESERVER_H

#define PORT 8080

#include "gamestate.h"
#include "socketthreadserver.h"
#include <map>
#include <list>
#include <QMutex>
#include <QTimer>
#include <QTcpServer>

typedef enum { OFF, WAIT, ON } Status;

class GameServer : public QTcpServer
{
public:
    GameServer();
    ~GameServer();

    void newConnection(int socketID); // Handle new connection according to current server status

private:
    Status m_ServerStatus;
    map<PlayerID, PlayerThreadServer*> m_socketThreadMap;
    list<SocketThreadServer> m_socketThreadList;
    QTimer m_timer;
    GameState m_gameState;
    QMutex m_gameStateMutex;         // Mutex is used to protect gamestate object

    void deleteSocket();
    void incomingData();
};

#endif // GAMESERVER_H

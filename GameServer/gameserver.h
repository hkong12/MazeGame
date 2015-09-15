#ifndef GAMESERVER_H
#define GAMESERVER_H

//#include "gamestate.h"
#include <QStringList>
#include <QTcpServer>
#include <QMutex>
#include <QTimer>

class GameServer : public QTcpServer
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};
    GameServer(QObject *parent = 0);

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

private:
    QStringList m_game;

    Status m_serverStatus;
    QTimer m_timer;
    QMutex m_gameStateMutex;
//    GameState m_gameState;
};

#endif // GAMESERVER_H

#include "gameserverthread.h"
#include "connection.h"
#include "gamestate.h"

#include <QtNetwork>

GameServerThread::GameServerThread(GameServer *server, Connection *conn, QThread *parent)
    : QThread(parent), m_server(server), m_conn(conn)
{
    connect(m_server, SIGNAL(gameStart()), this, SLOT(handleGameStart()));
    connect(m_conn, SIGNAL(newMove(QByteArray)), this, SLOT(handleNewMove(QByteArray)));
    connect(m_conn, SIGNAL(newAck()), this, SLOT(handleNewAck()));
}

void GameServerThread::handleGameStart()
{
    QByteArray message;
    m_server->getCurrentGameState(message);
    m_conn->sendMessage(Connection::GameState, message);
}

void GameServerThread::handleNewMove(QByteArray bytes)
{
    bool isbs = (bytes.mid(0,1) == "Y")? true:false;
    QString pid(bytes.mid(2,6));
    QString move(bytes.mid(9,1));
    QByteArray message;

    m_server->respondToMove(pid, move);
    m_server->getCurrentGameState(message);
    m_conn->sendMessage(Connection::GameState, message);
}

void GameServerThread::handleNewAck()
{
    m_server->setBackupServerTimeFlag(true);
    m_server->getBackupServerTimer()->stop();
}

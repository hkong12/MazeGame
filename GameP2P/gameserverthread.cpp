#include "gameserverthread.h"
#include "connection.h"
#include "gamestate.h"

#include <QtNetwork>

GameServerThread::GameServerThread(GameServer *server, Connection *conn, QThread *parent)
    : QThread(parent), m_server(server), m_conn(conn)
{
    connect(m_server, SIGNAL(gameStart()), this, SLOT(handleGameStart()));
    connect(m_conn, SIGNAL(newMove(QByteArray)), this, SLOT(handleNewMove(QByteArray)));
}

void GameServerThread::handleGameStart()
{
    QByteArray message;
    m_server->getCurrentGameState(message);
    m_conn->sendMessage(Connection::GameState, message);
}

void GameServerThread::handleNewMove(QByteArray bytes)
{
    QString pid(bytes.mid(0,6));
    QString move(bytes.mid(7,1));
    QByteArray message;
    m_server->respondToMove(pid, move);
    m_server->getCurrentGameState(message);
    m_conn->sendMessage(Connection::GameState, message);
}

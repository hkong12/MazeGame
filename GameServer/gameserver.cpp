#include "gameserver.h"
#include "gameserverthread.h"

#include <stdlib.h>

GameServer::GameServer(QObject *parent)
    :QTcpServer(parent)
{
    m_game << tr("Hello")
           << tr("Bye")
           << tr("Hahaha");
}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
    QString game = m_game.at(qrand() % m_game.size());
    GameServerThread *thread = new GameServerThread(socketDescriptor, game, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

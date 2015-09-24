#include "gameserverthread.h"

#include <QtNetwork>

GameServerThread::GameServerThread(GameServer* server, int socketDiscriptor, QObject *parent)
    :QObject(parent), m_socketDiscriptor(socketDiscriptor), m_serverPtr(server)
{
    m_connection = new Connection(Connection::Server);
    if(!m_connection->setSocketDescriptor(m_socketDiscriptor)) {
        emit error(m_connection->error());
        return;
    }

    m_connection->moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(finished()), m_connection, SLOT(deleteLater()));
    connect(m_serverPtr, SIGNAL(gameStart()), this, SLOT(handleGameStart()));
    connect(m_connection, SIGNAL(doneTcpSocket()), this, SLOT(quitThread()));
    connect(m_connection, SIGNAL(newClient()), this, SLOT(handleNewClient()));
    connect(m_connection, SIGNAL(newMove(QString&,const QString&)), this, SLOT(handleNewMove(QString&,const QString&)));
    m_thread.start();
}

GameServerThread::~GameServerThread() {
    m_thread.quit();
    m_thread.wait();
}

void GameServerThread::handleNewClient() {
    QString playerID = m_serverPtr->newClient();
    QString message;

    if(playerID.length() == 0) {
        message = "<Rejected> Current game is under way. Please try later...";
        m_connection->sendMessage(Connection::PlainText, message);
    } else {
        message = '<'+ playerID + '>' + " You have joined the new game. Please wait for other players...";
        m_connection->sendMessage(Connection::PlainText, message);
    }
}

void GameServerThread::handleNewMove(QString &playerID, const QString &move) {
    bool result = m_serverPtr->newMove(playerID, move);
    if(result == true) {
        // TODO
    } else {
        QString message = "Your current move is forbidden";
        m_connection->sendMessage(Connection::PlainText, message);
    }
}

void GameServerThread::handleGameStart() {
    // TODO: get current game status
    QString message = "";
    m_connection->sendMessage(Connection::GameState, message);
}

void GameServerThread::handleDoneTcpSocket() {
   m_thread.quit();
   m_thread.wait();
}

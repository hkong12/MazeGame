#include <QtNetwork>

#include "gameclient.h"
#include "gamestate.h"
#include "connection.h"

GameClient::GameClient()
{
    m_myPlayerID = "";
    m_status = OFF;
    m_gameState = NULL;
    m_connection = NULL;
}

GameClient::~GameClient()
{
    delete m_gameState;
    delete m_connection;
}

void GameClient::connectToServer(QString host, int port)
{
    m_connection = new Connection(Connection::Client);
    m_connection->connectToHost(host, port);

    connect(m_connection, SIGNAL(newGreeting(QByteArray)), this, SLOT(handleNewGreeting(QByteArray)));
    connect(m_connection, SIGNAL(newState(QByteArray)), this, SLOT(handleNewState(QByteArray)));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(handleGameOver()));
}

void GameClient::handleNewGreeting(QByteArray greeting)
{
    bool ok;
    greeting.mid(1, 6).toInt(&ok, 16);
    if(ok) {
        m_myPlayerID = greeting.mid(1,6);
        m_gameState = new GameState;
        emit initGameState(m_gameState);
        emit newClientInfo(m_myPlayerID+",0,WAIT");
    } else {
        m_connection->disconnectFromHost();
    }

    emit newLog(QString(greeting));
}

void GameClient::handleNewState(QByteArray state)
{
    m_gameState->readInitByteArray(state);
    QString treasure = QString::number(m_gameState->getPlayerTreasureNum(m_myPlayerID));
    QString isFinish = (m_gameState->getIsFinish())?"OFF":"ON";
    emit newClientInfo(m_myPlayerID+','+treasure+','+isFinish);
    if(m_gameState->getIsFinish()) {
        m_connection->disconnectFromHost();
        emit gameOver();
    }
}

void GameClient::handleTryMove(QString move)
{
    m_connection->sendMessage(Connection::Direction, QString(m_myPlayerID+','+move).toUtf8());
}

void GameClient::handleGameOver()
{
//    m_status = OFF;
//    m_myPlayerID = "";
//    delete m_connection;
}

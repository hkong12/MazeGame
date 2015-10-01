#include <QtNetwork>

#include "gameclient.h"
#include "gamestate.h"
#include "connection.h"
#include "peermanager.h"
#include "gameserver.h"

GameClient::GameClient(QString psIp, int psPort)
{
    m_myPlayerID = "";
    m_status = OFF;
    m_gameState = NULL;
    m_connection = NULL;

    GameServer* primaryServer = new GameServer(GameServer::PrimaryServer);
    if(primaryServer->listen(QHostAddress(psIp), psPort)) {
        m_isServer = true;
        QThread* sthread = new QThread;
        primaryServer->setParent(0);
        primaryServer->moveToThread(sthread);
        sthread->start();
    } else {
        m_isServer = false;
        delete primaryServer;
    }

    m_peerManager = new PeerManager();
    m_peerManager->setHasBackupServer(false);
    m_peerManager->setPrimaryServerAddr(psIp, psPort);
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
    connect(m_connection, SIGNAL(newBackupServer(QByteArray)), this, SLOT(handleNewBackupServer(QByteArray)));
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

void GameClient::handleNewBackupServer(QByteArray bserver)
{
    int tmp = bserver.lastIndexOf('>');
    QString bs = QString(bserver.mid(1, tmp-1));
    QString bsaddr = bs.section(',', 0, 0);
    QString bsport = bs.section(',', 1, 1);

    if(!m_isServer) {
        GameServer* backupServer = new GameServer(GameServer::BackupServer);
        backupServer->listen(QHostAddress(bsaddr), bsport.toInt());
        QThread* cthread = new QThread;
        backupServer->setParent(0);
        backupServer->moveToThread(cthread);
        cthread->start();
        m_isServer = false;
        m_connection->sendMessage(Connection::Acknowledge, QString('<' + bs + '>' + " has been selected as the backup server.").toUtf8());
        emit newLog(QString(bserver));
    }
}

void GameClient::handleTryMove(QString move)
{
    QString s = m_isServer? "Y" : "N";
    m_connection->sendMessage(Connection::Direction, QString(s+','+m_myPlayerID+','+move).toUtf8());
}

void GameClient::handleGameOver()
{
//    m_status = OFF;
//    m_myPlayerID = "";
//    delete m_connection;
}

#include <QtNetwork>

#include "gameclient.h"
#include "gamestate.h"
#include "connection.h"
#include "peermanager.h"
#include "gameserver.h"

static const int MoveTimerOut = 800;
static const int WaitGreetingTimeOut = 300;
static const int StartBindPort = 50000;

GameClient::GameClient(QString psIp, int psPort)
{
    m_myPlayerID = "";
    m_status = OFF;
    m_gameState = NULL;
    m_connection = NULL;
    m_connBindPort = -1;
    m_lastMove = "";
    m_moveTimer.setSingleShot(true);
    m_waitGreetingTimer.setSingleShot(true);

    connect(&m_moveTimer, SIGNAL(timeout()), this, SLOT(handleMoveTimerout()));
    connect(&m_waitGreetingTimer, SIGNAL(timeout()), this, SLOT(handleWaitGreetingTimerout()));

    // run for being the primary server
    GameServer* primaryServer = new GameServer(GameServer::PrimaryServer);
    if(primaryServer->listen(QHostAddress(psIp), psPort)) {
        m_isServer = true;
        QThread* sthread = new QThread;
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
    delete m_peerManager;
}

void GameClient::connectToServer(QString host, int port)
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(ipAddressesList.at(i) != QHostAddress::LocalHost &&
           ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }

    m_connection = new Connection(Connection::Client);

    // bind the connection to a certain port
    int i = 0;
    bool ok;
    do {
        i ++;
        ok = m_connection->bind(QHostAddress(ipAddress), StartBindPort+1000*i);
    } while(!ok);
    m_connBindPort = StartBindPort + 1000 * i;
    m_connection->connectToHost(host, port);
    // each connection use the same port as first time

    connect(m_connection, SIGNAL(newGreeting(QByteArray)), this, SLOT(handleNewGreeting(QByteArray)));
    connect(m_connection, SIGNAL(newState(QByteArray)), this, SLOT(handleNewState(QByteArray)));
    connect(m_connection, SIGNAL(newBackupServer(QByteArray)), this, SLOT(handleNewBackupServer(QByteArray)));
    connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connection, SLOT(sendMessage(Connection::DataType,QByteArray)));

    emit newPrimaryServerInfo(host+','+QString::number(port));
}

void GameClient::handleNewGreeting(QByteArray buffer)
{
    bool ok;
    // check it is rejected or accepted and get player id
    buffer.mid(1, 6).toInt(&ok, 16);
    if(ok) {
        m_myPlayerID = buffer.mid(1,6);
        m_gameState = new GameState;
        // update gui
        emit initGameState(m_gameState);
        emit newClientInfo(m_myPlayerID+",0,WAIT");
    } else {
        m_connection->disconnectFromHost();
    }

    // add log
    emit newLog(QString(buffer));
}

void GameClient::handleNewState(QByteArray buffer)
{
    // turn off the moveTimer if it is working
    if(m_moveTimer.isActive()) {
        m_moveTimer.stop();
    }

    // update peermanager use info within first {} of state
    int idx = buffer.indexOf('}');
    if(idx != -1) {
        QString serverInfo(buffer.mid(1, idx-1));
        QString bsip = serverInfo.section(',', 0, 0);
        int bsport = serverInfo.section(',', 1, 1).toInt();
        m_peerManager->setBackupServerAddr(bsip, bsport);
        m_peerManager->setHasBackupServer(true);
        emit newBackupServerInfo(serverInfo+','+bsport);
    }

    // use rest data update GameState
    QByteArray stateInfo = buffer.mid(idx+1);
    m_gameState->readInitByteArray(stateInfo);
    QString treasure = QString::number(m_gameState->getPlayerTreasureNum(m_myPlayerID));
    QString isFinish = (m_gameState->getIsFinish())?"OFF":"ON";
    emit newClientInfo(m_myPlayerID+','+treasure+','+isFinish);
    // check wether the game is finished
    if(m_gameState->getIsFinish()) {
        emit gameOver();
        emit newLog('<'+m_myPlayerID+'>'+ " Game is over.");
    }
}

void GameClient::handleNewBackupServer(QByteArray buffer)
{
    // get backup server ip and port
    int tmp = buffer.lastIndexOf('>');
    QString bs = QString(buffer.mid(1, tmp-1));
    QString bsaddr = bs.section(',', 0, 0);
    QString bsport = bs.section(',', 1, 1);

    // if client process does not maintain a server
    if(!m_isServer) {
        GameServer* backupServer = new GameServer(GameServer::BackupServer);
        backupServer->listen(QHostAddress(bsaddr), bsport.toInt());
        QThread* sthread = new QThread;
        backupServer->moveToThread(sthread);
        sthread->start();
        m_isServer = true;
        //emit haveMessageToSend(Connection::Acknowledge, QString('<' + bs + '>' + " has been selected as the backup server.").toUtf8());
        emit newLog(QString(buffer));
    }
}

void GameClient::handleTryMove(QString move)
{
    // transfer the move to server through connection
    QString s = m_isServer? "Y" : "N";
    disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
    connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connection, SLOT(sendMessage(Connection::DataType,QByteArray)));
    emit haveMessageToSend(Connection::Direction, QString(s+','+m_myPlayerID+','+move).toUtf8());
    // start timer for reply
    // when timeout and no reply resend to backup server
    m_moveTimer.start(MoveTimerOut);
    m_lastMove = move;
}

void GameClient::handleMoveTimerout() {
    // update backup server to primary server
    if(m_peerManager->getHasBackupServer()) {
        QString psip = m_peerManager->getBackupServerIp();
        int psport = m_peerManager->getBackupServerPort();
        m_peerManager->setPrimaryServerAddr(psip, psport);
        m_peerManager->setHasBackupServer(false);
        // connect to new primary server and resend last move to new primary server
        m_connection->close();
        m_connection->bind(m_connBindPort);
        m_connection->connectToHost(QHostAddress(psip), psport);

        // wait until tcp socket is writabel
        while(!m_connection->isWritable()) {
            QCoreApplication::processEvents();
        }
        m_waitGreetingTimer.start(WaitGreetingTimeOut);
    }
}

void GameClient::handleWaitGreetingTimerout()
{
    QString s = m_isServer? "Y" : "N";
    QString psip = m_peerManager->getBackupServerIp();
    int psport = m_peerManager->getBackupServerPort();
    emit haveMessageToSend(Connection::Direction, QString(s+','+m_myPlayerID+','+m_lastMove).toUtf8());
    emit newPrimaryServerInfo(psip+','+QString::number(psport));
    emit newBackupServerInfo(" , ");
    emit newLog("<Primary Server> You have been selected as primary server.");
}

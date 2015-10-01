#include "gameserver.h"
#include "gamestate.h"
#include "gameserverthread.h"
#include "connection.h"
#include "peermanager.h"

#include <QTime>

static const int WaitingTimeout = 20 * 1000;
static const int BackupServerTimeout = 3 * 1000;

GameServer::GameServer(ServerType st, QObject *parent)
    :QTcpServer(parent), m_servertType(st)
{
    m_serverStatus = OFF;
    m_playerList.clear();
    m_playerConnectionMap.clear();
    m_playerThreadMap.clear();
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_gameState = NULL;

    m_backupServerTimer = new QTimer(this);
    m_backupServerTimer->setSingleShot(true);
    m_backupServerTimerFlag = false;
    m_connToBackup = NULL;
    m_backupServerIp = "";
    m_backupServerPort = -1;

    connect(m_timer, SIGNAL(timeout()), this, SLOT(handleWaitingTimeout()));
}

GameServer::~GameServer()
{
    delete m_timer;
    delete m_backupServerTimer;
    if(m_connToBackup)
        delete m_connToBackup;
}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
    Connection::Identity cid = (m_servertType == PrimaryServer)? Connection::PrimaryServer : Connection::BackupServer;
    Connection* newcon = new Connection(cid);
    if(!newcon->setSocketDescriptor(socketDescriptor)) {
        emit socketError(newcon->error());
    } else if(cid == Connection::PrimaryServer) {
        connect(newcon, SIGNAL(newClient(Connection*)), this, SLOT(handleNewClient(Connection*)));
        connect(newcon, SIGNAL(newAck()), this, SLOT(handleNewAck()));
    } else if(cid == Connection::BackupServer) {
        connect(newcon, SIGNAL(newState(Connection*,QByteArray)), this, SLOT(handleNewState(Connection*,QByteArray)));
        connect(newcon, SIGNAL(newMove(Connection*,QByteArray)), this, SLOT(handleNewMove(Connection*,QByteArray)));
    }
}

void GameServer::getRandString(QString &randString)
{
    int max = 6;
    QString tmp = QString("0123456789ABCDEF");
    QString str = QString();
    for(int i=0;i<max;i++) {
        int ir = qrand()%tmp.length();
        str[i] = tmp.at(ir);
    }
    randString = str;
}

void GameServer::handleWaitingTimeout()
{
    // Assume no timeout before gamestart;
    m_serverStatusMutex.lock();

    // Initialize the game state;
    m_serverStatus = ON;
    m_gameState = new GameState(10, 10, &m_playerList);

    m_connToBackup = new Connection(Connection::PrimaryServer);
    m_connToBackup->connectToHost(QHostAddress(m_backupServerIp), m_backupServerPort);
    connect(m_connToBackup, SIGNAL(connected()), this, SLOT(initBackupServer()));
    m_serverStatusMutex.unlock();

    // Wait for connected
    emit gameStart();
}

void GameServer::initBackupServer()
{
    QByteArray bytes;
    m_gameState->writeByteArray(bytes);
    m_connToBackup->sendMessage(Connection::GameState, bytes);
}



void GameServer::handleNewClient(Connection *conn)
{
    QString playerID = addClient();
    QString message;

    if(playerID.length() == 0) {
        message = "<Rejected> Current game is under way. Please try later...";
        conn->sendMessage(Connection::Greeting, message.toUtf8());
    } else {
        m_playerThreadMap[playerID] = new GameServerThread(this, conn);
        m_playerThreadMap[playerID]->start();
        message = '<'+ playerID + '>' + " You have joined the new game. Please wait for other players...";
        conn->sendMessage(Connection::Greeting, message.toUtf8());
    }

    if(m_backupServerPort == -1) {
        QString backupServerAddr = conn->peerAddress().toString();
        int backupServerPort = this->serverPort() + 1000;
        message = '<' + backupServerAddr + ',' + QString::number(backupServerPort) + '>'
                +" You are selected as the backup server.";
        conn->sendMessage(Connection::BServer, message.toUtf8());

        // Assume in first 20s no crash
        m_backupServerIp = backupServerAddr;
        m_backupServerPort = backupServerPort;
    }
}

QString GameServer::addClient()
{
    QString playerID = "";

    m_serverStatusMutex.lock();
    if(m_serverStatus == OFF) { // start a new game

        m_playerList.clear();
        getRandString(playerID);
        m_playerList.append(playerID);
        m_serverStatus = WAIT;
        m_timer->start(WaitingTimeout);

    } else if(m_serverStatus == WAIT) { // join the current game

        getRandString(playerID); // get a unic playerID
        int i = 0;
        while(i < m_playerList.size()) {
            for(i = 0; i < m_playerList.size(); i++) {
                if(m_playerList.at(i) == playerID) {
                    getRandString(playerID);
                    break;
                }
            }
        }
        m_playerList.append(playerID);

    }
    m_serverStatusMutex.unlock();

    return playerID;
}

bool GameServer::respondToMove(QString pid, QString move)
{
    bool ok;
    m_connToBackup->sendMessage(Connection::Direction, QString(pid+','+move).toUtf8());
    // wait for reply
    m_gameStateMutex.lock();
    ok = m_gameState->responseToPlayerMove(pid, move);
    m_gameStateMutex.unlock();
    if(m_gameState->getIsFinish()) {
        m_serverStatusMutex.lock();
        m_serverStatus = OFF;
        m_serverStatusMutex.unlock();
    }
    return ok;
}

void GameServer::getCurrentGameState(QByteArray &barray)
{
    m_gameStateMutex.lock();
    m_gameState->writeByteArray(barray);
    m_gameStateMutex.unlock();
}

void GameServer::handleNewState(Connection *conn, QByteArray state)
{
    m_gameState = new GameState();
    m_gameState->readInitByteArray(state);
    conn->sendMessage(Connection::Acknowledge, QByteArray("Current state update to backup server."));
}

void GameServer::handleNewMove(Connection *conn, QByteArray move)
{
    QString pid(move.mid(2,6));
    QString dir(move.mid(9,1));
    respondToMove(pid, dir);
    conn->sendMessage(Connection::Acknowledge, QByteArray("Current state update to backup server."));
}

void GameServer::handleNewAck()
{

}

#include "gameserver.h"
#include "gamestate.h"
#include "gameserverthread.h"
#include "connection.h"

#include <QTime>

static const int WaitingTimeout = 20 * 1000;

GameServer::GameServer(QObject *parent)
    :QTcpServer(parent)
{
    m_serverStatus = OFF;
    m_playerList.clear();
    m_playerConnectionMap.clear();
    m_playerThreadMap.clear();
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_gameState = NULL;

    connect(m_timer, SIGNAL(timeout()), this, SLOT(handleWaitingTimeout()));
}

GameServer::~GameServer()
{

}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
//    GameServerThread* thread = new GameServerThread(this, socketDescriptor, this);
    Connection* newcon = new Connection(Connection::Server);
    if(!newcon->setSocketDescriptor(socketDescriptor)) {
        emit socketError(newcon->error());
    } else {
        connect(newcon, SIGNAL(newClient(Connection*)), this, SLOT(handleNewClient(Connection*)));
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
    m_serverStatusMutex.lock();
    // TODO: initialize the game state;
    m_gameState = new GameState(10, 10, &m_playerList);
    m_serverStatus = ON;
    emit gameStart();
    m_serverStatusMutex.unlock();
}

void GameServer::handleNewClient(Connection *conn)
{
    QString playerID = addClient();
    QString message;

    if(playerID.length() == 0) {
        message = "<Rejected> Current game is under way. Please try later...";
        conn->sendMessage(Connection::Greeting, message.toUtf8());
    } else {
        m_playerConnectionMap[conn] = playerID;
        m_playerThreadMap[playerID] = new GameServerThread(this, conn);
        m_playerThreadMap[playerID]->start();
        message = '<'+ playerID + '>' + " You have joined the new game. Please wait for other players...";
        conn->sendMessage(Connection::Greeting, message.toUtf8());
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
    m_gameStateMutex.lock();
    ok = m_gameState->responseToPlayerMove(pid, move);
    m_gameStateMutex.unlock();
    return ok;
}

void GameServer::getCurrentGameState(QByteArray &barray)
{
    m_gameStateMutex.lock();
    m_gameState->writeByteArray(barray);
    m_gameStateMutex.unlock();
}

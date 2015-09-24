#include "gameserver.h"
#include "gameserverthread.h"
#include "connection.h"

#include <QTime>

static const int WaitingTimeout = 20 * 1000;

GameServer::GameServer(QObject *parent)
    :QTcpServer(parent)
{
    m_serverStatus = OFF;
    m_playerList.clear();
    m_timer = NULL;

    connect(m_timer, SIGNAL(timeout()), this, SLOT(handleWaitingTimeout()));
}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
    GameServerThread* thread = new GameServerThread(this, socketDescriptor, this);

}

void GameServer::getRandString(QString &randString)
{
    int max = 8;
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWZYZ");
    QString str = QString();
    QTime t;
    t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
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
    m_game = "Game start.";
    m_serverStatus = ON;
    m_serverStatusMutex.unlock();

}

QString GameServer::newClient()
{
    QString playerID = "";

    m_serverStatusMutex.lock();
    if(m_serverStatus == OFF) { // start a new game

        m_playerList.clear();
        getRandString(playerID);
        m_playerList.append(playerID);
        m_serverStatus = WAIT;
        m_timer = new QTimer(this);
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

bool GameServer::newMove(QString &playerID, const QString &move)
{


}

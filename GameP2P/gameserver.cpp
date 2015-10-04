#include <QCoreApplication>
#include <QDataStream>
#include <QtNetwork>
#include <QThread>

#include "gameserver.h"
#include "gamestate.h"
#include "connection.h"
#include "peermanager.h"


static const int WaitingTimeout = 40 * 1000;
static const int BackupServerTimeout = 500;
static const int ConnToBackupPort = 33333;

GameServer::GameServer(ServerType st, QObject *parent)
    :QTcpServer(parent), m_serverType(st)
{
    m_playerConnectionMap.clear();
    m_playerAddrMap.clear();

    if(m_serverType == PrimaryServer) {
        m_serverStatus = OFF;
    } else {
        m_serverStatus = ON;
    }
    m_playerList.clear();
    m_gameState = NULL;

    m_hasBackupServer = false;
    m_backupServerIp = "";
    m_backupServerPort = -1;
    m_backupServerTimer = NULL;
    m_connToBackup = NULL;
    m_connToPrimary = NULL;
    m_lastMoveRequest = "";
}

GameServer::~GameServer()
{
    delete m_gameState;
    delete m_connToPrimary;
    delete m_connToBackup;
}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
    Connection::Identity cid = (m_serverType == PrimaryServer)? Connection::PrimaryServer : Connection::BackupServer;
    Connection* newcon = new Connection(cid);

    if(!newcon->setSocketDescriptor(socketDescriptor)) {
        emit socketError(newcon->error());
        return;
    }

//    connect(newcon, SIGNAL(newAck()), this, SLOT(handleNewAck()));
    connect(newcon, SIGNAL(newClient(Connection*)), this, SLOT(handleNewClient(Connection*)));
    connect(newcon, SIGNAL(newState(QByteArray)), this, SLOT(handleNewState(QByteArray)));
    connect(newcon, SIGNAL(newMove(QByteArray)), this, SLOT(handleNewMove(QByteArray)));
    connect(newcon, SIGNAL(newPlayerAddr(QByteArray)), this, SLOT(handleNewPlayerAddr(QByteArray)));

//    if(m_serverType == BackupServer && m_connToPrimary == NULL) {
//        m_connToPrimary = newcon;

//bug        QThread* sthread = new QThread;
//bug        newcon->moveToThread(sthread);
//bug        sthread->start();
//bug        connect(newcon, SIGNAL(destroyed()), sthread, SLOT(deleteLater()));
}

void GameServer::handleNewClient(Connection *conn)
{

    if(m_serverType == BackupServer && conn->peerPort() == ConnToBackupPort) {
        m_connToPrimary = conn;
    } else if(m_serverType == BackupServer && conn->peerPort() != ConnToBackupPort) {
        // update the backupserver to primary server
        m_serverType = PrimaryServer;
        connect(this, SIGNAL(upgradeConnection()), conn, SLOT(upgradeIdentityToPrimary()));
        emit upgradeConnection();
    }

    if(m_serverType == BackupServer)
        return;

    // check in the player address list first for the primary server that has just upgraded from a backup server
    // if it is in the list that mean the client has join the game in a privous server
    QString peerIp = conn->peerAddress().toString();
    int peerPort = conn->peerPort();
    QMap<QString, QPair<QString, int> >::iterator iter;
    for(iter = m_playerAddrMap.begin(); iter != m_playerAddrMap.end(); iter ++) {
        if(iter.value().first == peerIp && iter.value().second == peerPort) {
            m_playerConnectionMap[iter.key()] = conn;
//bug            QThread* cthread = new QThread;
//bug            conn->moveToThread(cthread);
//bug            cthread->start();
//bug            connect(conn, SIGNAL(destroyed()), cthread, SLOT(deleteLater()));
            return;
        }
    }

    // for the real new client
    QString playerId;
    bool result = addClient(playerId);
    QString message;

    if(result == false) {
        message = "<Rejected> Current game is under way. Please try later...";
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), conn, SLOT(sendMessage(Connection::DataType,QByteArray)));
        emit haveMessageToSend(Connection::Greeting, message.toUtf8());
    } else {
        m_playerConnectionMap[playerId] = conn;
        m_playerAddrMap[playerId] = QPair<QString, int>(peerIp, peerPort);
        // send message to the specific Connection
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), conn, SLOT(sendMessage(Connection::DataType,QByteArray)));
        message = '<'+ playerId + '>' + " You have joined the new game. Please wait for other players...";
        emit haveMessageToSend(Connection::Greeting, message.toUtf8());
        // run connection in new thread
//bug        QThread* cthread = new QThread;
//bug        conn->moveToThread(cthread);
//bug        cthread->start();
//bug        connect(conn, SIGNAL(destroyed()), cthread, SLOT(deleteLater()));

        // if primary server does not have a backup server
        if(!m_hasBackupServer) {
            // select new client as backup server
            QString backupServerAddr = conn->peerAddress().toString();
            int backupServerPort = this->serverPort() + 1000;
            disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
            connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), conn, SLOT(sendMessage(Connection::DataType,QByteArray)));
            message = '<' + backupServerAddr + ',' + QString::number(backupServerPort) + '>'
                    +" is selected as the backup server.";
            emit haveMessageToSend(Connection::SelectServer, message.toUtf8());
            // assume no crash in first 20s
            m_hasBackupServer = true;
            m_backupServerIp = backupServerAddr;
            m_backupServerPort = backupServerPort;
        }
    }
}


bool GameServer::addClient(QString &playerId)
{
    playerId.clear();

    // the mutex will be unlocked when locker is destroyed
    QMutexLocker locker(&m_serverStatusMutex);

    if(m_serverStatus == OFF) { // start a new game

        m_playerList.clear();
        getRandString(playerId);
        m_playerList.append(playerId);
        m_serverStatus = WAIT;
        m_gameStartTimer = new QTimer;
        m_gameStartTimer->setSingleShot(true);
        connect(m_gameStartTimer, SIGNAL(timeout()), this, SLOT(handleStartGameTimeout()));
        m_gameStartTimer->start(WaitingTimeout);
        return true;

    } else if(m_serverStatus == WAIT) { // join the current game

        // get a unic playerId
        getRandString(playerId);
        int i = 0;
        while(i < m_playerList.size()) {
            for(i = 0; i < m_playerList.size(); i++) {
                if(m_playerList.at(i) == playerId) {
                    getRandString(playerId);
                    break;
                }
            }
        }
        m_playerList.append(playerId);
        return true;
    }

    return false;
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

void GameServer::handleStartGameTimeout()
{
    QMutexLocker locker(&m_serverStatusMutex);

    // Initialize the game state;
    m_serverStatus = ON;
    m_gameState = new GameState(10, 10, &m_playerList);

    // initialize the backup server
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
    // connection to backup server is bind to certain port
    m_connToBackup = new Connection(Connection::PrimaryServer);
    m_connToBackup->bind(QHostAddress(ipAddress), ConnToBackupPort);
    m_connToBackup->connectToHost(QHostAddress(m_backupServerIp), m_backupServerPort);
    connect(m_connToBackup, SIGNAL(newAck()), this, SLOT(handleNewAck()));
    // bytes of serialized player address map
    QByteArray addrmap;
    QDataStream * stream = new QDataStream(&addrmap, QIODevice::WriteOnly);
    (*stream) << m_playerAddrMap;
    delete stream;
    // bytes of game state */
    QByteArray state;
    m_gameState->writeByteArray(state);
    while(!m_connToBackup->isWritable()) {
        QCoreApplication::processEvents();
    }
    disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
    connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connToBackup, SLOT(sendMessage(Connection::DataType,QByteArray)));
    emit haveMessageToSend(Connection::PlayerAddr, addrmap);
    emit haveMessageToSend(Connection::GameState, state);

    // send to all joined clients
    Connection* playerConnection;
    disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
    foreach(playerConnection, m_playerConnectionMap) {
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), playerConnection, SLOT(sendMessage(Connection::DataType,QByteArray)));
    }
    emit haveMessageToSend(Connection::GameState, state);

    m_backupServerTimer = new QTimer;
    m_backupServerTimer->setSingleShot(true);
    connect(m_backupServerTimer, SIGNAL(timeout()), this, SLOT(handleBackupServerTimeout()));
}

void GameServer::handleNewMove(QByteArray buffer)
{

    bool requestClientIsServer = (buffer.mid(0,1) == "Y")? true : false;
    QString pid = QString(buffer.mid(2, 6));
    QString move = QString(buffer.mid(9, 1));

    // If it is a backup server, send the ack to primary server
    if(m_serverType == BackupServer) {
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connToPrimary, SLOT(sendMessage(Connection::DataType,QByteArray)));
        emit haveMessageToSend(Connection::Acknowledge, QByteArray("Current movement update to backup server."));
        m_gameStateMutex.lock();
        m_gameState->responseToPlayerMove(pid, move);
        m_gameStateMutex.unlock();
        return;
    }

    // If it is a primary server, update to backup server and wait for reply
    if(m_serverType == PrimaryServer) {
        if(m_hasBackupServer) {
            disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
            connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connToBackup, SLOT(sendMessage(Connection::DataType,QByteArray)));
            emit haveMessageToSend(Connection::Direction, buffer);
            m_lastMoveRequest = buffer;
            m_backupServerTimer->start(BackupServerTimeout);
        } else {
            m_lastMoveRequest = buffer;
            handleBackupServerTimeout();
        }
    }
}

void GameServer::handleBackupServerTimeout()
{
    if(!m_backupServerTimer) {
        m_backupServerTimer = new QTimer;
        m_backupServerTimer->setSingleShot(true);
        connect(m_backupServerTimer, SIGNAL(timeout()), this, SLOT(handleBackupServerTimeout()));
    }

    bool requestClientIsServer = (m_lastMoveRequest.mid(0,1) == "Y")? true : false;
    QString pid = QString(m_lastMoveRequest.mid(2, 6));
    QString move = QString(m_lastMoveRequest.mid(9, 1));

    // when do not have a backup server
    QString selectedPlayerId = "";
    if(!requestClientIsServer) {
        // select request client as backup server
        selectedPlayerId = pid;
    } else {
        // select from player list
//        QMap<QString, Connection*>::iterator iter;
//        for(iter = m_playerConnectionMap.begin(); iter != m_playerConnectionMap.end(); iter ++) {
//            if(iter.value()->isWritable() && iter.key() != pid) {
//                selectedPlayerId = iter.key();
//                break;
//            }
//        }
    }

    if(selectedPlayerId.length() > 0) {
        QString backupServerIp = m_playerConnectionMap[selectedPlayerId]->peerAddress().toString();
        int backupServerPort = this->serverPort() + 1000;
        QString message = '<' + backupServerIp + ',' + QString::number(backupServerPort) + '>'
            +" is selected as the backup server.";
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_playerConnectionMap[selectedPlayerId], SLOT(sendMessage(Connection::DataType,QByteArray)));
        emit haveMessageToSend(Connection::SelectServer, message.toUtf8());

        // initialize the backup server
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
        if(m_connToBackup) {
            m_connToBackup->close();
        } else {
            m_connToBackup = new Connection(Connection::PrimaryServer);
        }
        m_connToBackup->bind(QHostAddress(ipAddress), ConnToBackupPort);
        m_connToBackup->connectToHost(QHostAddress(backupServerIp), backupServerPort);
        connect(m_connToBackup, SIGNAL(newAck()), this, SLOT(handleNewAck()));
        // bytes of serialized player address map
        QByteArray addrmap;
        QDataStream * stream = new QDataStream(&addrmap, QIODevice::WriteOnly);
        (*stream) << m_playerAddrMap;
        delete stream;
        // bytes of game state
        QByteArray state;
        m_gameState->writeByteArray(state);
        while(!m_connToBackup->isWritable()) {
            QCoreApplication::processEvents();
        }
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connToBackup, SLOT(sendMessage(Connection::DataType,QByteArray)));
        emit haveMessageToSend(Connection::PlayerAddr, addrmap);
        emit haveMessageToSend(Connection::GameState, state);
        emit haveMessageToSend(Connection::Direction, m_lastMoveRequest);

        // update backup server info
        m_hasBackupServer = true;
        m_backupServerIp = backupServerIp;
        m_backupServerPort = backupServerPort;

    } else {
        m_hasBackupServer = false;
        m_backupServerIp = "";
        m_backupServerPort = -1;
    }

    // update the game state on primary server and respond to client
    m_gameStateMutex.lock();
    m_gameState->responseToPlayerMove(pid, move);
    m_gameStateMutex.unlock();

    if(m_gameState->getIsFinish()) {
        m_serverStatusMutex.lock();
        m_serverStatus = OFF;
        m_serverStatusMutex.unlock();
    }

    if(m_serverType == PrimaryServer) {
        QByteArray bytes;
        // get current game state
        m_gameState->writeByteArray(bytes);
        // add backup server address
        QByteArray tmp;
        tmp.setNum(m_backupServerPort);
        if(m_hasBackupServer)
            bytes = '{' + m_backupServerIp.toUtf8() + ',' + tmp + '}' + bytes;
        disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
        connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_playerConnectionMap[pid], SLOT(sendMessage(Connection::DataType,QByteArray)));
        emit haveMessageToSend(Connection::GameState, bytes);
    }
}

void GameServer::handleNewState(QByteArray state)
{
    // for backup server
    m_gameState = new GameState();
    m_gameState->readInitByteArray(state);
//    disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
//    connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_connToPrimary, SLOT(sendMessage(Connection::DataType,QByteArray)));
//    emit haveMessageToSend(Connection::Acknowledge, QByteArray("Current state update to backup server."));
}

void GameServer::handleNewPlayerAddr(QByteArray buffer)
{
    QDataStream * stream = new QDataStream(&buffer, QIODevice::ReadOnly);
    (*stream) >> m_playerAddrMap;
    delete stream;

    QMap<QString, QPair<QString, int> >::iterator iter;
    for(iter = m_playerAddrMap.begin(); iter != m_playerAddrMap.end(); iter ++) {
        m_playerList.append(iter.key());
    }
}

void GameServer::handleNewAck()
{
    if(m_backupServerTimer != NULL && m_backupServerTimer->isActive()) {
        m_backupServerTimer->stop();

        bool requestClientIsServer = (m_lastMoveRequest.mid(0,1) == "Y")? true : false;
        QString pid = QString(m_lastMoveRequest.mid(2, 6));
        QString move = QString(m_lastMoveRequest.mid(9, 1));

        m_gameStateMutex.lock();
        m_gameState->responseToPlayerMove(pid, move);
        m_gameStateMutex.unlock();

        if(m_gameState->getIsFinish()) {
            m_serverStatusMutex.lock();
            m_serverStatus = OFF;
            m_serverStatusMutex.unlock();
        }

        if(m_serverType == PrimaryServer) {
            QByteArray bytes;
            // get current game state
            m_gameState->writeByteArray(bytes);
            // add backup server address
            QByteArray tmp;
            tmp.setNum(m_backupServerPort);
            bytes = '{' + m_backupServerIp.toUtf8() + ',' + tmp + '}' + bytes;
            disconnect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), 0, 0);
            connect(this, SIGNAL(haveMessageToSend(Connection::DataType,QByteArray)), m_playerConnectionMap[pid], SLOT(sendMessage(Connection::DataType,QByteArray)));
            emit haveMessageToSend(Connection::GameState, bytes);
        }
    }
}

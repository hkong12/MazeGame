/**************************************************************************/
/* Class GameServer                                                       */
/* This is the server-side of the Maze Game implemented with 2 kind, prim */
/* -ary server and backup server. As a primary server response to GameClient */
/* through Connection and maintain a backup server with latest GameState   */
/* info. As a backup server, keep the local GameState with the latest, and */
/* upgrad to primary server when previous primary server crashes.          */
/**************************************************************************/

#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>
#include <QList>
#include <QMap>

#include "connection.h"

class GameState;


class GameServer : public QTcpServer
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};
    enum ServerType {PrimaryServer, BackupServer};
    GameServer(ServerType st, QObject *parent = 0);
    ~GameServer();

    bool respondToMove(QString pid, QString move);

signals:
    // signal to Connection
    void haveMessageToSend(Connection::DataType, QByteArray message);
    void upgradeConnection();
    // signal for error
    void socketError(QTcpSocket::SocketError);

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

private slots:
    void handleNewClient(Connection* conn);
    void handleNewState(QByteArray buffer);
    void handleNewMove(QByteArray buffer);
    void handleNewPlayerAddr(QByteArray buffer);
    void handleNewAck();

    void handleStartGameTimeout();
    void handleBackupServerTimeout();

private:
    void getRandString(QString &str);
    bool addClient(QString &playerId);

    // for server itself
    ServerType m_serverType;
    QMap<QString, Connection*> m_playerConnectionMap;
    // a map of player id with connection address
    QMap<QString,QPair<QString, int> > m_playerAddrMap;

    // for game
    QTimer* m_gameStartTimer;
    QMutex m_serverStatusMutex;
    Status m_serverStatus;
    QList<QString> m_playerList;
    QMutex m_gameStateMutex;
    GameState* m_gameState;


    // for it's backup server
    bool m_hasBackupServer;
    QString m_backupServerIp;
    int m_backupServerPort;
    QTimer* m_backupServerTimer;
    Connection* m_connToBackup;
    Connection* m_connToPrimary;
    QByteArray m_lastMoveRequest;
};

#endif // GAMESERVER_H

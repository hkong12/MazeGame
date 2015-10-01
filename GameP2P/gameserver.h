#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>
#include <QList>
#include <QMap>

class GameState;
class Connection;
class GameServerThread;
class PeerManager;

class GameServer : public QTcpServer
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};
    enum ServerType {PrimaryServer, BackupServer};
    GameServer(ServerType st, QObject *parent = 0);
    ~GameServer();

    bool respondToMove(QString pid, QString move);
    void getCurrentGameState(QByteArray &barray);
    void setBackupServerTimeFlag(bool b) { m_backupServerTimerFlag = b; }
    QTimer* getBackupServerTimer() { return m_backupServerTimer; }

signals:
    void socketError(QTcpSocket::SocketError);
    void gameStart();

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

private slots:
    void handleWaitingTimeout();
    void initBackupServer();
    void handleNewClient(Connection* conn);
    void handleNewState(Connection*, QByteArray state);
    void handleNewMove(Connection*, QByteArray move);
    void handleNewAck();

private:
    void getRandString(QString &str);
    QString addClient();

    QMap<Connection*, QString> m_playerConnectionMap;
    QMap<QString, GameServerThread*> m_playerThreadMap;
    Status m_serverStatus;
    QTimer *m_timer;
    QMutex m_serverStatusMutex;
    QMutex m_gameStateMutex;
    QList<QString> m_playerList;
    GameState* m_gameState;

    ServerType m_servertType;
    QString m_backupServerIp;
    int m_backupServerPort;
    QTimer* m_backupServerTimer;
    bool m_backupServerTimerFlag;
    Connection* m_connToBackup;
};

#endif // GAMESERVER_H

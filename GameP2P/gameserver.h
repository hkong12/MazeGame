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

class GameServer : public QTcpServer
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};
    GameServer(QObject *parent = 0);
    ~GameServer();

    bool newMove(QString &playerID, const QString &move);
    bool respondToMove(QString pid, QString move);
    void getCurrentGameState(QByteArray &barray);

signals:
    void socketError(QTcpSocket::SocketError);
    void gameStart();

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;

private slots:
    void handleWaitingTimeout();
    void handleNewClient(Connection* conn);
//    void handleNewMove();

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
};

#endif // GAMESERVER_H

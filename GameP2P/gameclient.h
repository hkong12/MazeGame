/**************************************************************************/
/* Class GameClient                                                       */
/* This is the client-side of the Maze Game with 3 main functions: 1.Res  */
/* -ponse player's input and send it to Connection; 2. Receive info from  */
/* GameServer through Connection to update local GameState. 3. In peer-to */
/* -peer structure, generate new backup server when necessary.            */
/**************************************************************************/

#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTimer>

#include "connection.h"

class GameState;
class GameServer;
class PeerManager;

class GameClient : public QObject
{
    Q_OBJECT

public:
    enum Status {OFF, WAIT, ON};

    GameClient(QString psIp, int psPort);
    ~GameClient();
    void connectToServer(QString host, int port);
    Status getClientStatus() { return m_status; }
    GameState *getGameState() { return m_gameState; }
    bool getIsServer() { return m_isServer; }

signals:
    // signals to Connection
    void haveMessageToSend(Connection::DataType, QByteArray message);

    // signals to update GUI
    void newClientInfo(QString info);
    void newPrimaryServerInfo(QString info);
    void newBackupServerInfo(QString info);
    void newLog(QString log);
    void initGameState(GameState* gs);
    void gameOver();

public slots:
    // slot to handle signals emitted by Connection
    void handleNewGreeting(QByteArray buffer);
    void handleNewState(QByteArray buffer);
    void handleNewBackupServer(QByteArray buffer);

    // slot to handle signals emitted by GUI
    void handleTryMove(QString move);

private slots:
    // slot to handl timer out
    void handleMoveTimerout();
    void handleWaitGreetingTimerout();

private:
    Status m_status;
    QString m_myPlayerID;
    GameState *m_gameState;
    Connection *m_connection;
    int m_connBindPort;
    PeerManager* m_peerManager;

    // marked when the client process has selected to maintain a server
    bool m_isServer;

    // timer and cache for player movement
    QTimer m_moveTimer;
    QTimer m_waitGreetingTimer;
    QString m_lastMove;
};

#endif // GAMECLIENT_H

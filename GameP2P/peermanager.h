#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <QObject>
#include <QHostAddress>

class GameServer;

class PeerManager : public QObject
{
    Q_OBJECT

public:
    PeerManager() {}
    PeerManager(QString psIp, int psPort);

    void setPrimaryServerAddr(QString ip, int port) { m_primaryServerIp = ip; m_primaryServerPort = port; }
    void setBackupServerAddr(QString ip, int port) { m_backupServerIp = ip; m_backupServerPort = port; }

    QString getPrimaryServerIp() { return m_primaryServerIp; }
    QString getBackupServerIp() { return m_backupServerIp; }
    int getPrimaryServerPort() { return m_primaryServerPort; }
    int getBackupServerPort() { return m_backupServerPort; }
    bool getIsPrimaryServer() { return m_isPrimaryServer; }
    bool getIsBackupServer() { return m_isBackupServer; }
    void setIsPrimaryServer(bool b) { m_isPrimaryServer = b; }
    void setIsBackupServer(bool b) { m_isBackupServer = b; }

private:
    bool m_isPrimaryServer;
    bool m_isBackupServer;
    QString m_primaryServerIp;
    int m_primaryServerPort;
    QString m_backupServerIp;
    int m_backupServerPort;

    GameServer* m_primaryServer;
    GameServer* m_backupServer;
};

#endif // PEERMANAGER_H

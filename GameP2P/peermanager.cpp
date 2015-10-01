#include <QThread>
#include "peermanager.h"
#include "gameserver.h"

PeerManager::PeerManager(QString psIp, int psPort)
    : m_primaryServerIp(psIp), m_primaryServerPort(psPort)
{
    m_isBackupServer = false;
    m_backupServerIp = "";
    m_backupServerPort = -1;
    m_backupServer = NULL;

    m_primaryServer = new GameServer;
    if(m_primaryServer->listen(QHostAddress(m_primaryServerIp), m_primaryServerPort)) {
        m_isPrimaryServer = true;

        QThread* sthread = new QThread;
        m_primaryServer->setParent(0);
        m_primaryServer->moveToThread(sthread);
        sthread->start();

    } else {
        delete m_primaryServer;
        m_primaryServer = NULL;
        m_isPrimaryServer = false;
    }
}


#ifndef SOCKETTHREADBASE_H
#define SOCKETTHREADBASE_H

#include <QString>
#include <QMutex>
#include <QTcpSocket>

class SocketThreadBase
{
public:
    SocketThreadBase( const QString &ipAddr, const ushort port );
    enum Identity { BAD_IDENTITY, SERVER, CLIENT };
    virtual void startThread() = 0;
    virtual void stopThread() = 0;
    virtual Identity getIdentity() = 0;

protected:
    QMutex m_mutex;
    const QString m_IP;
    const ushort m_port;

    void setRunThread( bool newVal );
    bool getRunThread();
    void run();
    QString readLine( QTcpSocket *socket );
    int waitForInput( QTcpSocket *socket );
    void writeLine( QTcpSocket *client, const QString &line );

private:
    bool m_runThread;
};

#endif // SOCKETTHREADBASE_H

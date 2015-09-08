#ifndef SOCKETTHREADSERVER_H
#define SOCKETTHREADSERVER_H

#include "socketthreadbase.h"
#include <QThread>

class SocketThreadServer : public SocketThreadBase, QThread
{
public:
    SocketThreadServer( const QString &ipAddr, const ushort port );
    ~SocketThreadServer();
    void startThread();
    void stopThread();
    Identity getIdentity() { return SERVER; }

protected:
    void run();
    void echoServer( QTcpSocket *client );

private:

};

#endif // SOCKETTHREADSERVER_H

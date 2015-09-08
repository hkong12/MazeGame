#ifndef SOCKETTHREADCLIENT_H
#define SOCKETTHREADCLIENT_H

#include "socketthreadbase.h"
#include <QThread>

class SocketThreadClient : public SocketThreadBase, QThread
{
public:
    SocketThreadClient(const QString &ipAddr, const ushort port );
    ~SocketThreadClient();
    void startThread();
    void stopThread();
    bool isDown();
    Identity getIdentity() { return CLIENT; }

protected:
    void run();
    void setDone ( const bool newVal );

private:
    static const unsigned int CONNECT_TIME_OUT = 5 * 1000; // 5 seconds
    static const char *m_stringsp[];
    static const uint m_numStrings;
    bool mDown;
};

#endif // SOCKETTHREADCLIENT_H

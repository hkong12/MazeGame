#ifndef GAMESERVERTHREAD_H
#define GAMESERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>

class GameServerThread : public QThread
{
    Q_OBJECT

public:
    GameServerThread(int socketDescriptor, const QString &text, QObject *parent);
    void run() Q_DECL_OVERRIDE;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    int m_socketDescriptor;
    QString m_text;
};

#endif // GAMESERVERTHREAD_H

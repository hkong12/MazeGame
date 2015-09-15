#include "gameserverthread.h"

#include <QtNetwork>

GameServerThread::GameServerThread(int socketDescriptor, const QString &text, QObject *parent)
    :QThread(parent), m_socketDescriptor(socketDescriptor), m_text(text)
{
}

void GameServerThread::run()
{
    QTcpSocket tcpSocket;
    if(!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << m_text;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    tcpSocket.write(block);

    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
}


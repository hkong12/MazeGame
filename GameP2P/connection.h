#ifndef CONNECTION_H
#define CONNECTION_H

#include <QTcpSocket>

static const int MaxBufferSize = 1024000;

class Connection : public QTcpSocket
{
    Q_OBJECT


public:
    enum DataType {
        Greeting,
        Direction,
        GameState,
        SelectServer,
        Acknowledge,
        PlayerAddr,
        Undefined
    };
    enum Identity {
        PrimaryServer,
        BackupServer,
        Client
    };

    Connection(Identity identity, QObject *parent = 0);

signals:
    // signals to all
    void newGreeting(QByteArray greeting);

    // signals to both primary server and backup server
    void newMove(QByteArray buffer); // when client send to primary server or primary server update to backup server

    // signals to both backup server and client
    void newState(QByteArray buffer); // when primary server send to backup server or to client

    // signals to primary server
    void newClient(Connection* conn); // when client send greeting to primary server
    void newAck(); // when backup server sendback to primary server

    // signals to client
    void newBackupServer(QByteArray buffer); // when primary server send slect backup server to client

    // signals to backup server
    void newPlayerAddr(QByteArray buffer); // when primary server send to backup server for initialization

protected:
    void timerEvent(QTimerEvent *timerEvent);

public slots:
    void sendMessage(Connection::DataType dataType, QByteArray message);
    void upgradeIdentityToPrimary();

private slots:
    void processReadyRead();
    void sendGreetingMessage();

private:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();

    Identity m_identity;
    DataType m_currentDataType;
    QByteArray m_buffer;
    int m_transferTimerId;
    int m_numBytesForCurrentDataType;

};

#endif // CONNECTION_H

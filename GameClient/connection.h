#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
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
        PlainText,
        Undefined
    };
    enum Identity {
        Server,
        Client
    };

    Connection(Identity identity, QObject *parent = 0);
    void setGreetingMessage();
    bool sendMessage(DataType dataType, const QByteArray &message);

signals:
    void newGreeting(QByteArray greeting);
    void newClient(Connection* conn);
    void newMove(QByteArray move);
    void newState(QByteArray state);

    void delTcpSocket();

protected:
    void timerEvent(QTimerEvent *timerEvent);

private slots:
    void processReadyRead();
    void sendGreetingMessage();
    void doneTcpSocket();

private:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processDataServer();
    void processDataClient();

    Identity m_identity;
    DataType m_currentDataType;
    QByteArray m_buffer;
    int m_transferTimerId;
    int m_numBytesForCurrentDataType;

};

#endif // CONNECTION_H

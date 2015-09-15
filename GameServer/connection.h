#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>
#include <QTcpSocket>

static const int MaxBufferSize = 1024000;

class Connection : QTcpSocket
{
    Q_OBJECT

public:
    enum ConnectionState {

    };
    enum DataType {
        Greeting,
        Direction,
        GameState,
        PlainText,
        Undefined
    };

    Connection(QObject *parent = 0);
    void sendGreetingMessage();
    bool sendMessage(DataType dataType, const QString &message);

protected:
    void timerEvent(QTimerEvent *timerEvent);

private slots:
    void processReadyRead();

private:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();

    static QString sm_playerID;
    QString m_greetingMessage;
    DataType currentDataType;
    QByteArray m_buffer;
    int m_transferTimerId;
    int m_numBytesForCurrentDataType;

};

#endif // CONNECTION_H

#include "connection.h"
#include <QtNetwork>

static const int TransferTimeout = 30 * 1000;
static const char SeparatorToken = ' ';

Connection::Connection(Identity identity, QObject *parent )
    : QTcpSocket(parent)
{
    m_currentDataType = Undefined;
    m_transferTimerId = 0;
    m_numBytesForCurrentDataType = -1;
    m_identity = identity;

    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    QObject::connect(this, SIGNAL(connected()), this, SLOT(sendGreetingMessage()));
}

void Connection::sendGreetingMessage()
{
    if(m_identity == Client) {
        QString greetingMessage = "Join the game!";
        QByteArray greeting = greetingMessage.toUtf8();
        QByteArray data = "GREETING " + QByteArray::number(greeting.size()) + ' ' + greeting;
        if(write(data) != data.size())
            abort();
    }
}

bool Connection::sendMessage(DataType dataType, const QByteArray &message)
{
    if(message.isEmpty())
        return false;

    QString header;
    switch(dataType) {
    case Connection::Greeting:
        header = "GREETING ";
        break;
    case Connection::Direction:
        header = "DIRECTION ";
        break;
    case Connection::GameState:
        header = "GAMESTATE ";
        break;
    case Connection::PlainText:
        header = "PLAINTEXT ";
        break;
    case Connection::BServer:
        header = "BACKUPSERVER ";
        break;
    case Connection::Acknowledge:
        header = "ACKNOWLEDGE ";
        break;
    default:
        header = "UNDEFINED ";
        break;
    }

    QByteArray data = header.toUtf8() + QByteArray::number(message.size()) + ' ' + message;
    return write(data) == data.size();
}

void Connection::timerEvent(QTimerEvent *timerEvent)
{
    if(timerEvent->timerId() == m_transferTimerId) {
        abort();
        killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }
}

void Connection::doneTcpSocket()
{
}

void Connection::processReadyRead()
{
    if(m_identity == PrimaryServer) {

        if(!isValid()) {
            abort();
            return;
        }

        do {
            if(m_currentDataType == Undefined) {
                if(!readProtocolHeader())
                    return;
            }
            if(!hasEnoughData())
                return;
            processDataPrimaryServer();
        } while(bytesAvailable() > 0);

    }

    if(m_identity == BackupServer) {

        if(!isValid()) {
            abort();
            return;
        }

        do {
            if(m_currentDataType == Undefined) {
                if(!readProtocolHeader())
                    return;
            }
            if(!hasEnoughData())
                return;
            processDataBackupServer();
        } while(bytesAvailable() > 0);

    }

    if(m_identity == Client) {

        do {
            if(m_currentDataType == Undefined) {
                if(!readProtocolHeader())
                    return;
            }
            if(!hasEnoughData())
                return;
            processDataClient();
        } while(bytesAvailable() > 0);
    }

}

int Connection::readDataIntoBuffer(int maxSize)
{
    if(maxSize > MaxBufferSize)
        return 0;

    int numBytesBeforeRead = m_buffer.size();
    if(numBytesBeforeRead == MaxBufferSize) {
        abort();
        return 0;
    }

    while(bytesAvailable() > 0 && m_buffer.size() < maxSize) {
        m_buffer.append(read(1));
        if(m_buffer.endsWith(SeparatorToken))
            break;
    }
    return m_buffer.size() - numBytesBeforeRead;
}

int Connection::dataLengthForCurrentDataType()
{
    if(bytesAvailable() <= 0 || readDataIntoBuffer() <= 0
            || !m_buffer.endsWith(SeparatorToken))
        return 0;

    m_buffer.chop(1);
    int number = m_buffer.toInt();
    m_buffer.clear();
    return number;
}

bool Connection::readProtocolHeader()
{
    if(m_transferTimerId) {
        killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }

    if(readDataIntoBuffer() <= 0) {
        m_transferTimerId = startTimer(TransferTimeout);
        return false;
    }

    if(m_buffer == "GREETING ") {
        m_currentDataType = Greeting;
    } else if(m_buffer == "DIRECTION ") {
        m_currentDataType = Direction;
    } else if(m_buffer == "GAMESTATE ") {
        m_currentDataType = GameState;
    } else if(m_buffer == "PLAINTEXT ") {
        m_currentDataType = PlainText;
    } else if(m_buffer == "BACKUPSERVER ") {
        m_currentDataType = BServer;
    } else if(m_buffer == "ACKNOWLEDGE ") {
        m_currentDataType = Acknowledge;
    } else {
        m_currentDataType = Undefined;
        abort();
        return false;
    }

    m_buffer.clear();
    m_numBytesForCurrentDataType = dataLengthForCurrentDataType();
    return true;
}

bool Connection::hasEnoughData()
{
    if(m_transferTimerId) {
        QObject::killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }

    if(m_numBytesForCurrentDataType <= 0)
        m_numBytesForCurrentDataType = dataLengthForCurrentDataType();

    if(bytesAvailable() < m_numBytesForCurrentDataType
            || m_numBytesForCurrentDataType <= 0) {
        m_transferTimerId = startTimer(TransferTimeout);
        return false;
    }

    return true;
}

void Connection::processDataPrimaryServer()
{
    m_buffer = read(m_numBytesForCurrentDataType);
    if(m_buffer.size() != m_numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch(m_currentDataType) {
    case Greeting:
        emit newClient(this);
        break;
    case Direction:
        emit newMove(m_buffer);
        break;
    case Acknowledge:
        emit newAck();
        break;
    default:
        break;
    }

    m_currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}

void Connection::processDataBackupServer()
{
    m_buffer = read(m_numBytesForCurrentDataType);
    if(m_buffer.size() != m_numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch(m_currentDataType) {
    case GameState:
        emit newState(this, m_buffer);
        break;
    case Direction:
        // check where is it from
        emit newMove(this, m_buffer);
        break;
    default:
        break;
    }

    m_currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}

void Connection::processDataClient()
{
    m_buffer = read(m_numBytesForCurrentDataType);
    if(m_buffer.size() != m_numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch(m_currentDataType) {
    case BServer:
        emit newBackupServer(m_buffer);
        break;
    case Greeting:
        emit newGreeting(m_buffer);
        break;
    case PlainText:
        // Display
        break;
    case GameState:
        emit newState(m_buffer);
        break;
    default:
        break;
    }

    m_currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}

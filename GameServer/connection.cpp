#include "connection.h"

#include <QtNetwork>

static const int TransferTimeout = 30 * 1000;
static const char SeparatorToken = ' ';

Connection::Connection(QObject *parent)
    : QTcpSocket(parent)
{
    currentDataType = Undefined;
    m_transferTimerId = 0;
    m_numBytesForCurrentDataType = -1;
}

bool Connection::sendMessage(DataType dataType, const QString &message)
{
    if(message.isEmpty())
        return false;

    QString header;
    switch(dataType) {
    case Greeting:
        header = "GTREETING ";
        break;
    case Direction:
        header = "DIRECTION ";
        break;
    case GameState:
        header = "GAMESTATE ";
        break;
    case PlainText:
        header = "PLAINTEXT ";
        break;
    default:
        header = "UNDEFINED ";
        break;
    }

    QByteArray msg = message.toUtf8();
    QByteArray data = header.toUtf8() + QByteArray::number(msg.size()) + ' ' + msg;
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

    if(m_buffer == "GREETING") {
        currentDataType = Greeting;
    } else if(m_buffer == "DIRECTION") {
        currentDataType = Direction;
    } else if(m_buffer == "GAMESTATE") {
        currentDataType = GameState;
    } else if(m_buffer == "PLAINTEXT") {
        currentDataType = PlainText;
    } else {
        currentDataType = Undefined;
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

void Connection::processData()
{
    m_buffer = read(m_numBytesForCurrentDataType);
    if(m_buffer.size() != m_numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch(currentDataType) {
    case Greeting:
        break;
    case Direction:
        break;
    case GameState:
        break;
    default:
        break;
    }

    currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}

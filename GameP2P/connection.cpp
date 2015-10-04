#include <QtNetwork>

#include "connection.h"

static const int TransferTimeout = 30 * 1000;
static const char SeparatorToken = ' ';

Connection::Connection(Identity identity, QObject *parent )
    : QTcpSocket(parent)
{
    m_currentDataType = Undefined;
    m_transferTimerId = 0;
    m_numBytesForCurrentDataType = -1;
    m_identity = identity;
    m_buffer.clear();

    QObject::connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
    QObject::connect(this, SIGNAL(connected()), this, SLOT(sendGreetingMessage()));
}

void Connection::sendGreetingMessage()
{
        QString greetingMessage = "We are connected";
        QByteArray greeting = greetingMessage.toUtf8();
        QByteArray data = "GREETING " + QByteArray::number(greeting.size()) + ' ' + greeting;
        if(write(data) != data.size())
            abort();
}

void Connection::sendMessage(DataType dataType, QByteArray message)
{
    if(message.isEmpty())
        return;

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
    case Connection::SelectServer:
        header = "SELECTSERVER ";
        break;
    case Connection::Acknowledge:
        header = "ACKNOWLEDGE ";
        break;
    case Connection::PlayerAddr:
        header = "PLAYERADDR ";
        break;
    default:
        header = "UNDEFINED ";
        break;
    }

    QByteArray data = header.toUtf8() + QByteArray::number(message.size()) + ' ' + message;
    if(write(data) != data.size())
        abort();
    return;
}

void Connection::upgradeIdentityToPrimary()
{
    m_identity = PrimaryServer;
}

void Connection::timerEvent(QTimerEvent *timerEvent)
{
    if(timerEvent->timerId() == m_transferTimerId) {
        abort();
        killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }
}

void Connection::processReadyRead()
{
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

        processData();

    } while(bytesAvailable() > 0);
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
    } else if(m_buffer == "SELECTSERVER ") {
        m_currentDataType = SelectServer;
    } else if(m_buffer == "ACKNOWLEDGE ") {
        m_currentDataType = Acknowledge;
    } else if(m_buffer == "PLAYERADDR ") {
        m_currentDataType = PlayerAddr;
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

void Connection::processData()
{
    m_buffer = read(m_numBytesForCurrentDataType);
    if(m_buffer.size() != m_numBytesForCurrentDataType) {
        abort();
        return;
    }

    switch(m_currentDataType) {
    case Greeting:
        if(m_identity == PrimaryServer || m_identity == BackupServer) {
            emit newClient(this);
        } else if(m_identity == Client) {
            emit newGreeting(m_buffer);
        }
        break;
    case Direction:
        if(m_identity == PrimaryServer || m_identity == BackupServer) {
            emit newMove(m_buffer);
        }
        break;
    case GameState:
        if(m_identity == BackupServer || m_identity == Client) {
            emit newState(m_buffer);
        }
        break;
    case SelectServer:
        if(m_identity == Client) {
            emit newBackupServer(m_buffer);
        }
        break;
    case Acknowledge:
        if(m_identity == PrimaryServer) {
            emit newAck();
        }
        break;
    case PlayerAddr:
        if(m_identity == BackupServer) {
            emit newPlayerAddr(m_buffer);
        }
        break;
    default:
        break;
    }

    m_currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}

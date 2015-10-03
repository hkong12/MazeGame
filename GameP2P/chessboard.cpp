#include "chessboard.h"
#include "gamestate.h"

static const int BoardSize = 650;
static const int BorderSize = 20;

ChessBoard::ChessBoard(QWidget *parent) : QWidget(parent)
{
    setFixedSize(690, 690);
    m_gameState = NULL;
    m_playerID = "";
    m_paint = new QPainter;
}

void ChessBoard::setGameState(GameState *gameState)
{
    m_gameState = gameState;
    connect(m_gameState, SIGNAL(gameStateUpdated()), this, SLOT(updateChessBoard()));
    this->update();
}

void ChessBoard::setPlayerID(QString pid)
{
    m_playerID = pid;
}

void ChessBoard::paintEvent(QPaintEvent *)
{
    if(m_gameState == NULL || m_gameState->getSize() == -1) {
        m_paint->begin(this);
        m_paint->setPen(QPen(Qt::black,4,Qt::SolidLine));
        m_paint->drawRect(BorderSize, BorderSize, BoardSize, BoardSize);
        m_paint->end();
        return;
    }

    int blockNumber = m_gameState->getSize();
    int blockSize = BoardSize / blockNumber;
    int sideLength = blockNumber * blockSize;

    m_paint->begin(this);
    m_paint->setPen(QPen(Qt::black, 4, Qt::SolidLine));

    // Draw horizontal lines
    for(int i = 0; i < blockNumber+1; i++) {
        m_paint->drawLine(BorderSize, BorderSize+blockSize*i, BorderSize+sideLength, BorderSize+blockSize*i);
    }
    // Draw vertical lines
    for(int i = 0; i < blockNumber+1; i++) {
        m_paint->drawLine(BorderSize+blockSize*i, BorderSize, BorderSize+blockSize*i, BorderSize+sideLength);
    }

    bool res;
    std::pair<PlayerID, Location> pm;
    std::map<PlayerID, Location>* playerMap = m_gameState->getPlayerMap();
    foreach(pm, *playerMap) {
        PlayerID pid = pm.first;
        Location l = pm.second;
        QColor color(pid.mid(0, 2).toInt(&res, 16), pid.mid(2, 2).toInt(&res, 16), pid.mid(4, 2).toInt(&res, 16));
        if(pm.first == m_playerID) {
            m_paint->setPen(QPen(Qt::red, 2, Qt::SolidLine));
        } else {
            m_paint->setPen(QPen(Qt::white, 2, Qt::SolidLine));
        }
        m_paint->setBrush(QBrush(color, Qt::SolidPattern));
        m_paint->drawEllipse(BorderSize+(l.first+0.1)*blockSize, BorderSize+(l.second+0.1)*blockSize, blockSize*0.8, blockSize*0.8);
    }

    std::pair<Location, int> tm;
    std::map<Location, int>* treasureMap = m_gameState->getTreasureMap();
    foreach(tm, *treasureMap) {
        Location l = tm.first;
        m_paint->setPen(QPen(Qt::white, 2, Qt::SolidLine));
        m_paint->setBrush(QBrush(Qt::yellow, Qt::Dense1Pattern));
        m_paint->drawEllipse(BorderSize+(l.first+0.25)*blockSize, BorderSize+(l.second+0.25)*blockSize, blockSize*0.5, blockSize*0.5);
    }

    m_paint->end();

}

void ChessBoard::keyPressEvent(QKeyEvent *event)
{
    if(m_gameState == NULL || m_playerID.length() == 0)
        return;

    switch(event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        emit tryMove(QString("W"));
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        emit tryMove(QString("A"));
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        emit tryMove(QString("S"));
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        emit tryMove(QString("D"));
        break;
    default:
        break;
    }
}


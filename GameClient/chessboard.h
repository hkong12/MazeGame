#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QPainter>
#include <QKeyEvent>

class GameState;

class ChessBoard : public QWidget
{
    Q_OBJECT

public:
    ChessBoard(QWidget *parent = 0);
    void setGameState(GameState *gameState);
    void setPlayerID(QString pid);

signals:
    void tryMove(QString move);

public slots:
    void updateChessBoard() { this->update(); }

private:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    QString m_playerID;
    QPainter *m_paint;
    GameState *m_gameState;
};

#endif // CHESSBOARD_H

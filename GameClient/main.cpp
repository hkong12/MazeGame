#include <QApplication>
#include "gameui.h"
#include "client.h"
#include "gameclient.h"
#include "gamestate.h"
#include "chessboard.h"

void getRandString(QString &randString)
{
    int max = 6;
    QString tmp = QString("0123456789ABCDEF");
    QString str = QString();
    for(int i=0;i<max;i++) {
        int ir = qrand()%tmp.length();
        str[i] = tmp.at(ir);
    }
    randString = str;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qsrand(QTime::currentTime().msec());

    QList<QString> playerList;
    playerList.clear();
    QString str;
    for(int i = 0; i < 8; i++) {
        str.clear();
        getRandString(str);
        playerList.append(str);
    }

    GameState *gameState = new GameState(10, 10, &playerList);
 //   gameState->read();

    ChessBoard chessBoard;
    chessBoard.show();
    chessBoard.setGameState(gameState);
    chessBoard.setPlayerID(playerList.at(0));

//    GameUI game;
//    game.show();

    return app.exec();
}

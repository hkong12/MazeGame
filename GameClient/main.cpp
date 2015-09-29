#include <QApplication>
#include <iostream>
#include "gameui.h"
#include "connection.h"

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

    Connection client(Connection::Client);
    client.connectToHost("192.168.0.133", 30000);

/*
    QList<QString> playerList;
    playerList.clear();
    QString str;
    for(int i = 0; i < 8; i++) {
        str.clear();
        getRandString(str);
        playerList.append(str);
    }

    GameState *gameState = new GameState(10, 10, &playerList);
*/
//    GameUI game;
//    game.m_chessBoard->setGameState(gameState);
//    game.m_chessBoard->setPlayerID(playerList.at(0));
//    game.show();

    return app.exec();
}

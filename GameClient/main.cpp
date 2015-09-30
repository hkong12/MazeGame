#include <QApplication>
#include <iostream>
#include "gameclient.h"
#include "gameui.h"
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

    GameClient client;
//    client.connectToServer();

    GameUI game;
    game.setClient(&client);
//    game.m_chessBoard->setPlayerID(playerList.at(0));
    game.show();

    return app.exec();
}

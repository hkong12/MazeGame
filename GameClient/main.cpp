#include <QApplication>
#include "gameclient.h"
#include "gameui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qsrand(QTime::currentTime().msec());

    GameClient client;

    GameUI game;
    game.setClient(&client);
    game.show();

    return app.exec();
}

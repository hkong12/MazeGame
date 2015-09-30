#include <QApplication>
#include <QtNetwork>
#include <QThread>

#include "gameclient.h"
#include "gameserver.h"
#include "gameui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime::currentTime().msec());

    QThread* cthread = new QThread;
    GameClient client;
    client.moveToThread(cthread);

    GameUI game;
    game.setClient(&client);
    game.show();

    GameServer server;
    QString ipAddress;
    QHostAddress tmpAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(ipAddressesList.at(i) != QHostAddress::LocalHost &&
           ipAddressesList.at(i).toIPv4Address()) {
            tmpAddress = ipAddressesList.at(i);
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    server.listen(tmpAddress, 30000);
    QThread* sthread = new QThread;
    server.moveToThread(sthread);

    cthread->start();
    sthread->start();

    return app.exec();
}

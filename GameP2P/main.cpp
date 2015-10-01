#include <QApplication>
#include <QtNetwork>
#include <QThread>

#include "gameclient.h"
#include "peermanager.h"
#include "gameui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime::currentTime().msec());


    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(ipAddressesList.at(i) != QHostAddress::LocalHost &&
           ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    PeerManager* peerManager = new PeerManager(ipAddress, 30000);

    GameClient client;
    client.setPeerManager(peerManager);
    GameUI game;
    game.setClient(&client);
    game.show();

/*
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
    server.setParent(0);
    server.moveToThread(sthread);

 //   cthread->start();
    sthread->start();
*/

    return app.exec();
}

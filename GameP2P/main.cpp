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
    qRegisterMetaType<Connection::DataType>("Connection::DataType");

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

    GameClient client(ipAddress, 30000);
    GameUI game;
    game.setClient(&client);
    game.show();

    return app.exec();
}

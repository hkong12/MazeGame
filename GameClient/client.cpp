#include <QtWidgets>
#include <QtNetwork>

#include "client.h"

Client::Client(QWidget *parent)
    :QDialog(parent), networkSession(0)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("&Server port:"));

    hostCombo = new QComboBox;
    hostCombo->setEditable(true);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if(!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if(!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if(name != QString("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP address of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    //add localhost addresses
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit = new QLineEdit;
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("Please run the game server as well."));

    getTextButton = new QPushButton(tr("Get Text"));
    getTextButton->setDefault(true);
    getTextButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getTextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(hostCombo, SIGNAL(editTextChanged(QString)),
            this, SLOT(enableGetTextButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetTextButton()));
    connect(getTextButton, SIGNAL(clicked(bool)),
            this, SLOT(requestNewText()));
    connect(quitButton, SIGNAL(clicked(bool)),
            this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(readText()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Game Client"));
    portLineEdit->setFocus();

    QNetworkConfigurationManager manager;
    if(manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        getTextButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();

    }

}

void Client::requestNewText()
{
    getTextButton->setEnabled(false);
    blockSize = 0;
    tcpSocket->abort();

    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
}

void Client::readText()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if(blockSize == 0) {
        if(tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;

        in >> blockSize;
    }

    if(tcpSocket->bytesAvailable() < blockSize)
        return;

    QString nextText;
    in >> nextText;

    if(nextText == currentText) {
        QTimer::singleShot(0, this, SLOT(requestNewText()));
        return;
    }

    currentText = nextText;

    statusLabel->setText(currentText);
    getTextButton->setEnabled(true);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch(socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Game Client"),
                                 tr("The host was not found. Please check the "
                                 "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Game Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running,"
                                    "and check that the host name and port"
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Game Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getTextButton->setEnabled(true);
}

void Client::enableGetTextButton()
{
    getTextButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                              !hostCombo->currentText().isEmpty() &&
                              !portLineEdit->text().isEmpty());
}

void Client::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    statusLabel->setText(tr("Run the Game Server as well."));

    enableGetTextButton();
}


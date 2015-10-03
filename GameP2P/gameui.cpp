#include <QtNetwork>

#include "gameui.h"
#include "gameclient.h"
#include "gamestate.h"
#include "chessboard.h"

GameUI::GameUI(QWidget *parent) : QWidget(parent)
{
    m_client = NULL;
    m_chessBoard = new ChessBoard;

    m_hostLabel = new QLabel(tr("Server name:"));
    m_portLabel = new QLabel(tr("Server port:"));

    m_hostCombo = new QComboBox;
    m_hostCombo->setEditable(true);

    // Find out IP address of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for(int i = 0; i < ipAddressesList.size(); ++i) {
        if(!ipAddressesList.at(i).isLoopback())
            m_hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    m_portLineEdit = new QLineEdit;
    m_portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    m_hostLabel->setBuddy(m_hostCombo);
    m_portLabel->setBuddy(m_portLineEdit);

    m_startButton = new QPushButton(tr("Start Game"));
    m_startButton->setDefault(true);
    m_startButton->setEnabled(false);

    m_quitButton= new QPushButton(tr("Quit Game"));
    m_quitButton->setDefault(false);
    m_quitButton->setEnabled(true);

    connect(m_startButton, SIGNAL(clicked(bool)), this, SLOT(requestStart()));
    connect(m_quitButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(m_hostCombo, SIGNAL(editTextChanged(QString)), this, SLOT(enableStartButton()));
    connect(m_portLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableStartButton()));

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->addButton(m_startButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_quitButton, QDialogButtonBox::RejectRole);

    QGridLayout *controlPanelLayout = new QGridLayout;
    controlPanelLayout->addWidget(m_hostLabel, 0, 0);
    controlPanelLayout->addWidget(m_hostCombo, 0, 1);
    controlPanelLayout->addWidget(m_portLabel, 1, 0);
    controlPanelLayout->addWidget(m_portLineEdit, 1, 1);
    controlPanelLayout->addWidget(m_buttonBox, 2, 0, 1, 2);

    m_clientStatus = new QLabel(tr("Client Status:\nPlayer ID:\nTreasure Amount:\nStatus:\n"));
    m_clientStatus->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_clientStatus->setWordWrap(true);
    m_primaryServerStatus = new QLabel(tr("Primary Server Status:\nHostname:\nPort:\n"));
    m_primaryServerStatus->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_primaryServerStatus->setWordWrap(true);
    m_backupServerStatus = new QLabel(tr("Backup Server Status:\nHostname:\nPort:\n"));
    m_backupServerStatus->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_backupServerStatus->setWordWrap(true);

    m_logLabel = new QLabel(tr("Log Infomation:"));
    m_logTextEdit = new QTextEdit;
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFixedHeight(200);
    m_logLabel->setBuddy(m_logTextEdit);

    QGridLayout *statusPanelLayout = new QGridLayout;
    statusPanelLayout->addWidget(m_clientStatus, 0, 0);
    statusPanelLayout->addWidget(m_primaryServerStatus, 1, 0);
    statusPanelLayout->addWidget(m_backupServerStatus, 2, 0);
    statusPanelLayout->addWidget(m_logLabel, 3, 0);
    statusPanelLayout->addWidget(m_logTextEdit, 4, 0);

    QGridLayout *panelLayout = new QGridLayout;
    panelLayout->addLayout(controlPanelLayout, 0, 0);
    panelLayout->addLayout(statusPanelLayout, 1, 0);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(m_chessBoard, 0, 0);
    mainLayout->addLayout(panelLayout, 0, 1);
    this->setLayout(mainLayout);

    setWindowTitle(tr("Maze Game"));
    m_chessBoard->setFocus();
}

void GameUI::setClient(GameClient *client)
{
    m_client = client;

    connect(m_client, SIGNAL(newLog(QString)), this, SLOT(handleNewLog(QString)));
    connect(m_client, SIGNAL(newClientInfo(QString)), this, SLOT(handleNewClientInfo(QString)));
    connect(m_client, SIGNAL(newPrimaryServerInfo(QString)), this, SLOT(handleNewPrimaryServerInfo(QString)));
    connect(m_client, SIGNAL(newBackupServerInfo(QString)), this, SLOT(handleNewBackupServerInfo(QString)));
    connect(m_client, SIGNAL(initGameState(GameState*)), this, SLOT(handleInitGameState(GameState*)));
    connect(m_client, SIGNAL(gameOver()), this, SLOT(handleGameOver()));
    connect(m_chessBoard, SIGNAL(tryMove(QString)), m_client, SLOT(handleTryMove(QString)));

    if(client->getIsServer())
         m_logTextEdit->insertPlainText('\n' + QString("<Primary Server> You have been selected as primary server."));
}

void GameUI::requestStart()
{
    m_startButton->setEnabled(false);
    m_hostCombo->setEditable(false);
    m_portLineEdit->setEnabled(false);
    m_client->connectToServer(m_hostCombo->currentText(), m_portLineEdit->text().toInt());
    m_chessBoard->setFocus();
}

void GameUI::enableStartButton()
{
    m_startButton->setEnabled(!m_hostCombo->currentText().isEmpty()
                              && !m_portLineEdit->text().isEmpty()
                              && m_client->getClientStatus() == GameClient::OFF);
}

void GameUI::handleNewLog(QString log)
{
    m_logTextEdit->insertPlainText('\n'+log);
}

void GameUI::handleNewClientInfo(QString info)
{
    m_chessBoard->setPlayerID(info.section(',',0,0));
    QString t = QString(tr("Client Status:\nPlayer ID: %1\nTreasure Amount: %2\nStatus: %3\n"))
            .arg(info.section(',',0,0), info.section(',',1,1), info.section(',',2,2));
    m_clientStatus->setText(t);
}

void GameUI::handleNewPrimaryServerInfo(QString info)
{
    QString t = QString(tr("Primary Server Status:\nHostname: %1\nPort: %2\n"))
            .arg(info.section(',',0,0), info.section(',',1,1));
    m_primaryServerStatus->setText(t);
}

void GameUI::handleNewBackupServerInfo(QString info)
{
    QString t = QString(tr("Backup Server Status:\nHostname: %1\nPort: %2\n"))
            .arg(info.section(',',0,0), info.section(',',1,1));
    m_backupServerStatus->setText(t);
}

void GameUI::handleInitGameState(GameState *gs)
{
    m_chessBoard->setGameState(gs);
}

void GameUI::handleGameOver()
{
    m_hostCombo->setEditable(true);
    m_portLineEdit->setEnabled(true);
}

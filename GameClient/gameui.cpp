#include <QtNetwork>

#include "gameui.h"
#include "chessboard.h"

GameUI::GameUI(QWidget *parent) : QWidget(parent)
{
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
    m_primaryServerStatus = new QLabel(tr("Primary Server Status:\nHostname:\nPort:\nStatus:\n"));
    m_primaryServerStatus->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_primaryServerStatus->setWordWrap(true);
    m_backupServerStatus = new QLabel(tr("Backup Server Status:\nHostname:\nPort:\nStatus:\n"));
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
    m_portLineEdit->setFocus();

}

void GameUI::requestStart()
{

}

void GameUI::enableStartButton()
{

}


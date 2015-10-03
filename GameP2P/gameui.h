/**************************************************************************/
/* Class GameUI                                                           */
/* This is the main widget of the application, which display and update   */
/* the chessboard, client info, primary server info, backup server info   */
/* and log.                                                               */
/**************************************************************************/

#ifndef GAMEUI_H
#define GAMEUI_H

#include <QtWidgets>

class ChessBoard;
class GameClient;
class GameState;

class GameUI : public QWidget
{
    Q_OBJECT

public:
    GameUI(QWidget *parent = 0); 
    void setClient(GameClient* client);

public slots:
    void requestStart();
    void enableStartButton();
    void handleNewLog(QString log);
    void handleNewClientInfo(QString info);
    void handleNewPrimaryServerInfo(QString info);
    void handleNewBackupServerInfo(QString info);
    void handleInitGameState(GameState* gs);
    void handleGameOver();

private:
    QLabel *m_hostLabel;
    QLabel *m_portLabel;
    QComboBox *m_hostCombo;
    QLineEdit *m_portLineEdit;
    QPushButton *m_startButton;
    QPushButton *m_quitButton;
    QDialogButtonBox *m_buttonBox;

    QLabel *m_clientStatus;
    QLabel *m_primaryServerStatus;
    QLabel *m_backupServerStatus;

    QLabel *m_logLabel;
    QTextEdit *m_logTextEdit;

    GameClient *m_client;
    ChessBoard *m_chessBoard;
};

#endif // GAMEUI_H

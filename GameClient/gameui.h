#ifndef GAMEUI_H
#define GAMEUI_H

#include <QtWidgets>

class ChessBoard;

class GameUI : public QWidget
{
    Q_OBJECT

public:
    GameUI(QWidget *parent = 0);
    ChessBoard* getChessBoard() { return m_chessBoard; }

public slots:
    void requestStart();
    void enableStartButton();

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

    ChessBoard *m_chessBoard;
};

#endif // GAMEUI_H

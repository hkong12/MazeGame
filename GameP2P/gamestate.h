/**************************************************************************/
/* Class GameState                                                        */
/* This class stores current information about the maze game for each pla */
/* -yer and provide public function for handling player's instruction.    */
/**************************************************************************/

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QObject>
#include <QList>
#include <QJsonObject>

typedef std::pair<int, int> Location;
typedef QString PlayerID;

class GameState : public QObject
{
    Q_OBJECT

public:
    GameState();
    GameState(int n, int m, QList<QString> *playerList, QObject *parent = 0);
    bool responseToPlayerMove(PlayerID, QString);

    void writeByteArray(QByteArray &barray) const;
    void readInitByteArray(const QByteArray &barray);
    void readUpdateByteArray(const QByteArray &barray);

    int getSize() { return m_Size; }
    int getPlayerNumber() { return m_playerNumber; }
    std::map<PlayerID, Location>* getPlayerMap() { return &m_playerMap; }
    std::map<Location, int>* getTreasureMap() { return &m_TreasureMap; }
    int getPlayerTreasureNum(QString playerID) { return m_playerTreasureCount[playerID]; }
    bool getIsFinish() { return m_isFinish; }

signals:
    void gameStateError(QString str);
    void gameStateUpdated();
    void gameFinished();

public slots:

private:
    int m_Size;
    int m_TreasureCount;
    int m_playerNumber;
    bool m_isFinish;
    std::map<Location, int> m_TreasureMap;         // Record current treasure location and corresponding amount
    std::map<PlayerID, Location> m_playerMap;      // Record current player location
    std::map<PlayerID, int> m_playerTreasureCount; // Record current treasure amount of each player

    Location randomLocation();
    void write(QJsonObject &json) const;
    void initRead(const QJsonObject &json);
    void updateRead(const QJsonObject &json);
};

#endif // GAMESTATE_H

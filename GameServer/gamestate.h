#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <map>
#include <QList>

typedef std::pair<int, int> Location;
typedef QString PlayerID;

class GameState
{
public:
    GameState();
    GameState(int n, int m, QList<QString> *playerList);
    ~GameState();
    void responseToPlayerAction(PlayerID, QString);
    void prepareJSON(bool);

    int getSize() { return m_Size; }
    int getPlayerNumber() { return m_playerNumber; }

private:
    int m_Size;
    int m_TreasureCount;
    int m_playerNumber;
    QList<QString> * m_playerList;
    std::map<Location, int> m_TreasureMap;           // Record current treasure location and corresponding amount
    std::map<PlayerID, Location> m_playerMap;        // Record current player location
    std::map<PlayerID, int> m_playerTreasureCount; // Record current treasure amount of each player
    void GameInit();
    Location randomLocation();
};

#endif // GAMESTATE_H

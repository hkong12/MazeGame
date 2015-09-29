#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <map>
#include <QList>
#include <QJsonObject>

typedef std::pair<int, int> Location;
typedef QString PlayerID;

class GameState
{
public:
    GameState();
    GameState(int n, int m, QList<QString> *playerList);
    ~GameState();
    void responseToPlayerAction(PlayerID, QString);
    // read and write JSON method
    // read is used for P2P
    // write is used for both P2P and CS version
    void read(const QJsonObject &json);
    void write(const QJsonObject &json);

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
    void readMapLocationInt(const QJsonObject &json);
    void readMapPlayerIDInt(const QJsonObject &json);
    void readMapPlayerIDLocation(const QJsonObject &json);
    void writeMapLocationInt(const QJsonObject &json);
    void writeMapPlayerIDInt(const QJsonObject &json);
    void writeMapPlayerIDLocation(const QJsonObject &json);
    QString pairToString(Location);
    Location stringToPair(QString);
};

#endif // GAMESTATE_H

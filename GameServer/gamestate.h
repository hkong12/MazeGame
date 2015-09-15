#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <map>

typedef pair<int, int> Location;
typedef unsigned int PlayerID;

class GameState
{
public:
    GameState();
    ~GameState();

    int getMapWidth() { return sm_mapWidth; }
    int getMapLength() { return sm_mapLength; }
    int getPlayerNumber() { return sm_playerNumber; }

    initTreasureMap();
    lookupTreasureMap();
    updateTreasureMap();

    initPlayerMap();
    lookupPlayerMap();
    updatePlayerMap();

    initPlayerTreasureCount();
    lookupPlayerTreasureCount();
    updatePlayerTreasureCount();

    getGameStateToJson();   // Get current game state in JSON format

private:
    static int sm_mapWidth;
    static int sm_mapLength;
    static int sm_playerNumber;
    map<Location, int> m_TreasureMap;           // Record current treasure location and corresponding amount
    map<PlayerID, Location> m_playerMap;        // Record current player location
    map<PlayerID, int> m_playerTreasureCounter; // Record current treasure amount of each player
};

#endif // GAMESTATE_H

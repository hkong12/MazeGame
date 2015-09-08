#ifndef PLAYER_H
#define PLAYER_H

#include "gamestate.h"
#include "socketthreadclient.h"
#include <string>

typedef enum { OFF, WAIT, ON } Status;
typedef enum { EAST, SOUTH, WEST, NORTH, NOMOVE } Direction;

class Player
{
public:
    Player();
    ~Player();

    joinGame();
    Move( Direction );

private:
    PlayerID sm_myID;
    static std::string sm_hostName;
    static unsigned int sm_hostPort;

    Status m_myStatus;
    GameState m_myGameState;
    PlayerThreadClient m_myPlayerThreadClient;

    initMyGameState();
    updateMyGameState();
};

#endif // PLAYER_H

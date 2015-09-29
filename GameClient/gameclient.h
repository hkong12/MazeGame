#ifndef GAMECLIENT_H
#define GAMECLIENT_H

class GameState;

class GameClient
{

public:
    GameClient();

private:

    GameState *m_gameState;
};

#endif // GAMECLIENT_H

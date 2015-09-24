#include "gamestate.h"

GameState::GameState()
{

}

GameState::GameState(int n, int m, QList<QString> *playerList)
{
    m_Size = n;
    m_TreasureCount = m;
    m_playerNumber = playerList->size();
    GameInit();
}

Location GameState::randomLocation()
{
    // BE CAREFUL FOR not so random location
    int num, x, y;
    num = rand() % (m_Size * m_Size);
    x = num / m_Size;
    y = num % m_Size;
    return std::make_pair(x, y);
}

void GameState::GameInit()
{
    int i;
    bool locationEqual = false;
    srand(time(NULL));
    Location tmp;
    // set up treasure map
    for (i = 0; i < m_TreasureCount; i++)
    {
        tmp = randomLocation();
        if (m_TreasureMap.find(tmp) == m_TreasureMap.end())
            m_TreasureMap[tmp] = 1;
        else
            m_TreasureMap[tmp] = m_TreasureMap[tmp] + 1;
    }
    i = 0;
    // set up player map
    while (i < m_playerNumber)
    {
        tmp = randomLocation();
        for (std::map<PlayerID, Location>::iterator it=m_playerMap.begin();
             it !=m_playerMap.end() && locationEqual == false; ++ it)
        {
            if ((it-> second.first == tmp.first && it -> second.second == tmp.second) ||
                    m_TreasureMap.find(tmp) != m_TreasureMap.end())
                locationEqual = true;
        }
        if (locationEqual == true)
            continue;
        else
        {
            m_playerMap[m_playerList->at(i)] = tmp;
            ++i;
        }
    }
    // set up player treasure count
    for ( i = 0; i < m_playerNumber; ++i)
        m_playerTreasureCount[m_playerList->at(i)] = 0;

}

void GameState::responseToPlayerAction(PlayerID pi, QString move)
{
    Location currentLocation, possibleLocation;
    bool moveSuccessful = true;
    bool locationEqual = false;
    if (m_playerMap.find(pi) != m_playerMap.end())
        currentLocation = m_playerMap[pi];
    // TODO: if playerID is not in the map. then it is not a valid player
    /* Calculate the possible location according to move and current location */
    switch(move.at(0).unicode())
    {
    case 'W': {
        possibleLocation.second = currentLocation.second - 1;
        possibleLocation.first = currentLocation.first;
        break;
    }
    case 'S': {
        possibleLocation.second = currentLocation.second + 1;
        possibleLocation.first = currentLocation.first;
        break;
    }
    case 'A': {
        possibleLocation.second = currentLocation.second;
        possibleLocation.first = currentLocation.first - 1;
        break;
    }
    case 'D': {
        possibleLocation.second = currentLocation.second;
        possibleLocation.first = currentLocation.first + 1;
        break;
    }
    default:
        moveSuccessful = false;
    }
    /* check the validity of the possible location */
    /* check whether it is out of boundary */
    /* check whether it is in a occupied cell */
    if (moveSuccessful && possibleLocation.first < 0 || possibleLocation.first > m_Size - 1 ||
            possibleLocation.second < 0 || possibleLocation.second > m_Size - 1)
    {
        // TODO: The player hit the wall. Move is unsuccessful.
        moveSuccessful = false;
    }
    if (moveSuccessful)
    {
        for (std::map<PlayerID, Location>::iterator it = m_playerMap.begin();
             it !=m_playerMap.end() && locationEqual == false; ++it )
        {
            if ((it-> second.first == possibleLocation.first && it -> second.second == possibleLocation.second))
                locationEqual = true;
        }
    }
    if (locationEqual) moveSuccessful = false;
    if (moveSuccessful)
    {
        /* update playerID treasure count */
        /* update playerID location */
        /* clean up the treasure map */
        m_playerTreasureCount.find(pi)->second += m_TreasureMap.find(possibleLocation)->second;
        m_playerMap.find(pi)->second.first = possibleLocation.first;
        m_playerMap.find(pi)->second.second = possibleLocation.second;
        m_TreasureMap.erase(possibleLocation);
    }
    prepareJSON(moveSuccessful);
}

void GameState::prepareJSON(bool moveSuccessful)
{

}

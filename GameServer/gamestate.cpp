#include <QJsonArray>
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
    // TODO's: delete this prepareJSON(moveSuccessful);
}

void GameState::read(const QJsonObject &json)
{
    m_playerList->clear();
    m_TreasureMap.clear();
    m_playerMap.clear();
    m_playerTreasureCount.clear();

    m_Size = json["Size"].toInt();
    m_TreasureCount = json["TreasureCount"].toInt();
    m_playerNumber = json["playerNumber"].toInt();

    QJsonArray playerArray = json["playerList"].toArray();
    for (int playerIndex  = 0; playerIndex < playerArray.size(); ++ playerIndex) {
        QString playerStr = playerArray[playerIndex].toString();
        m_playerList->append(playerStr);
    }

    QJsonArray treasureMapArray = json["TreasureMap"].toArray();
    for (int treasureMapIndex = 0; treasureMapIndex < treasureMapArray.size(); ++ treasureMapIndex) {
        QJsonObject treasureMapObject = treasureMapArray[treasureMapIndex].toObject();
        readMapLocationInt(treasureMapObject);
    }

    QJsonArray playerMapArray = json["playerMap"].toArray();
    for (int playerMapIndex = 0; playerMapIndex < playerMapArray.size(); ++ playerMapIndex) {
        QJsonObject playerMapObject = playerMapArray[playerMapIndex].toObject();
        readMapPlayerIDLocation(playerMapObject);
    }

    QJsonArray playerTreasureCountArray = json["playerTreasureCount"].toArray();
    for (int playerTreasureCountIndex = 0; playerTreasureCountIndex < playerTreasureCountArray.size();
         ++ playerTreasureCountIndex) {
        QJsonObject playerTreasureCountObject = playerTreasureCountArray[playerTreasureCountIndex].toObject();
        readMapPlayerIDInt(playerTreasureCountObject);
    }
}

void GameState::write(const QJsonObject &json)
{
    json["Size"] = m_Size;
    json["TreasureCount"] = m_TreasureCount;
    json["playerNumber"] = m_playerNumber;

    QJsonArray playerArray;
    for (int i = 0; i < m_playerList->size(); ++i) {
        playerArray.append(m_playerList->at(i));
    }
    json["playerList"] = playerArray;

    writeMapLocationInt(json);
    writeMapPlayerIDInt(json);
    writeMapPlayerIDLocation(json);
}


void GameState::readMapLocationInt(const QJsonObject &json)
{
    int x, y, value;
    Location tmp;
    x = stringToPair(json.keys().at(0)).first;
    y = stringToPair(json.keys().at(0)).second;
    value = json.begin().value().toInt();
    tmp = std::make_pair(x, y);
    m_TreasureMap[tmp] = value;
}


void GameState::readMapPlayerIDInt(const QJsonObject &json)
{
    QString pi;
    int value;
    pi = json.keys().at(0);
    value = json.begin().value().toInt();
    m_playerTreasureCount[pi] = value;
}


void GameState::readMapPlayerIDLocation(const QJsonObject &json)
{
    QString pi;
    Location tmp;
    int x, y;
    pi = json.keys().at(0);
    x = stringToPair(json.begin().value().toString()).first;
    y = stringToPair(json.begin().value().toString()).second;
    tmp = std::make_pair(x, y);
    m_playerMap[pi] = tmp;
}


void GameState::writeMapLocationInt(const QJsonObject &json)
{
    QJsonArray treasureMapArray;
    for (std::map<Location, int>::iterator it = m_TreasureMap.begin();
         it != m_TreasureMap.end(); ++it ) {
        QJsonObject treasureMapObject;
        treasureMapObject[pairToString(it->first)] = it->second;
        treasureMapArray.append(treasureMapObject);
    }
    json["TreasureMap"] = treasureMapArray;
}


void GameState::writeMapPlayerIDInt(const QJsonObject &json)
{
    QJsonArray playerTreasureCountArray;
    for (std::map<PlayerID, int>::iterator it = m_playerTreasureCount.begin();
         it != m_playerTreasureCount.end(); ++it) {
        QJsonObject playerTreasureCountObject;
        playerTreasureCountObject[it->first] = it->second;
        playerTreasureCountArray.append(playerTreasureCountObject);
    }
    json["playerTreasureCount"] = playerTreasureCountArray;
}


void GameState::writeMapPlayerIDLocation(const QJsonObject &json)
{
    QJsonArray playerMapArray;
    for (std::map<PlayerID, Location>::iterator it = m_playerMap.begin();
         it != m_playerMap.end(); ++it ) {
        QJsonObject playerMapObject;
        playerMapObject[it->first] = pairToString(it->second);
        playerMapArray.append(playerMapObject);
    }
    json["playerMap"] = playerMapArray;
}

QString GameState::pairToString(Location a)
{
    return QString::number(a.first) + "," + QString::number(a.second);
}

Location GameState::stringToPair(QString a)
{
    return std::make_pair(a.split(",")[0].toInt(),a.split(",")[1].toInt());
}

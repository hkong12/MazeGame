#include <QJsonDocument>
#include "gamestate.h"

GameState::GameState()
{
    m_Size = -1;
    m_TreasureCount = -1;
    m_playerNumber = -1;
    m_isFinish = false;
    m_TreasureMap.clear();
    m_playerMap.clear();
    m_playerTreasureCount.clear();
}

GameState::GameState(int n, int m, QList<QString> *playerList, QObject *parent)
    :  QObject(parent), m_Size(n), m_TreasureCount(m)
{
    m_playerNumber = playerList->size();
    m_isFinish = false;
    m_TreasureMap.clear();
    m_playerMap.clear();
    m_playerTreasureCount.clear();

    // set up treasure map
    for (int i = 0; i < m_TreasureCount; i++) {
        Location tmp = randomLocation();
        if (m_TreasureMap.find(tmp) == m_TreasureMap.end())
            m_TreasureMap[tmp] = 1;
        else
            m_TreasureMap[tmp] = m_TreasureMap[tmp] + 1;
    }

    // set up player map
    int i = 0;
    while (i < m_playerNumber) {
        bool locationEqual = false;
        Location tmp = randomLocation();
        std::pair<PlayerID, Location> pm;
        foreach(pm, m_playerMap) {
            if (pm.second.first == tmp.first && pm.second.second == tmp.second) {
                locationEqual = true;
                break;
            }
        }
        if(m_TreasureMap.find(tmp) != m_TreasureMap.end()) {
            locationEqual = true;
        }
        if (locationEqual == false) {
            m_playerMap[playerList->at(i)] = tmp;
            i ++;
        }
    }
    // set up player treasure count
    for (int i = 0; i < m_playerNumber; ++i)
        m_playerTreasureCount[playerList->at(i)] = 0;

}

Location GameState::randomLocation()
{
    // BE CAREFUL FOR not so random location
    int num, x, y;
    num = qrand() % (m_Size * m_Size);
    x = num / m_Size;
    y = num % m_Size;
    return std::make_pair(x, y);
}

bool GameState::responseToPlayerMove(PlayerID pi, QString move)
{
    Location currentLocation, possibleLocation;
    if (m_playerMap.find(pi) != m_playerMap.end()) {
        currentLocation = m_playerMap[pi];
    } else {
        emit gameStateError(QString("The player ID: %1, is not in the player map.").arg(pi));
        return false;
    }

    // Calculate the possible location according to move and current location
    // Check whether it is out of range
    switch(move.at(0).unicode())
    {
    case 'W': {
        possibleLocation.second = currentLocation.second - 1;
        possibleLocation.first = currentLocation.first;
        if(possibleLocation.second < 0)
            return false;
        break;
    }
    case 'S': {
        possibleLocation.second = currentLocation.second + 1;
        possibleLocation.first = currentLocation.first;
        if(possibleLocation.second > m_Size-1)
            return false;
        break;
    }
    case 'A': {
        possibleLocation.second = currentLocation.second;
        possibleLocation.first = currentLocation.first - 1;
        if(possibleLocation.first < 0)
            return false;
        break;
    }
    case 'D': {
        possibleLocation.second = currentLocation.second;
        possibleLocation.first = currentLocation.first + 1;
        if(possibleLocation.first > m_Size-1)
            return false;
        break;
    }
    default:
        return false;
    }

    // Check whether it is in a occupied cell
    std::pair<PlayerID, Location> pm;
    foreach(pm, m_playerMap) {
        if(pm.second.first == possibleLocation.first && pm.second.second == possibleLocation.second)
            return false;
    }

    // Move successfully
    // Update the game state
    // update playerID treasure count, playerID location, clean up the treasure map
    if(m_TreasureMap.find(possibleLocation) != m_TreasureMap.end()) {
        if(m_playerTreasureCount.find(pi) != m_playerTreasureCount.end()) {
            m_playerTreasureCount.find(pi)->second += m_TreasureMap.find(possibleLocation)->second;
            m_TreasureMap.erase(possibleLocation);
            // Check whether the game is finished
            if(m_TreasureMap.size() == 0) {
                m_isFinish = true;
                emit gameFinished();
            }
        } else {
            emit gameStateError(QString("The player ID: %1, is not in the player-treasure count map.").arg(pi));
            return false;
        }
    }
    m_playerMap.find(pi)->second.first = possibleLocation.first;
    m_playerMap.find(pi)->second.second = possibleLocation.second;

    //TEST: emit signal for chess board
    emit gameStateUpdated();

    return true;
}

void GameState::initRead(const QJsonObject &json)
{
    m_TreasureMap.clear();
    m_playerMap.clear();
    m_playerTreasureCount.clear();

    m_Size = json["Size"].toInt();
    m_TreasureCount = json["TreasureCount"].toInt();
    m_playerNumber = json["playerNumber"].toInt();

    updateRead(json);
}

void GameState::updateRead(const QJsonObject &json)
{
    QString item, a, b;
    m_isFinish = json["isFinish"].toBool();

    QJsonObject treasureMap = json["TreasureMap"].toObject();
    foreach(item, treasureMap.keys()) {
        QString tmp = item.mid(1);
        tmp.chop(1);
        a = tmp.section(',', 0, 0);
        b = tmp.section(',', 1, 1);
        Location l = std::make_pair(a.toInt(), b.toInt());
        m_TreasureMap[l] = treasureMap[item].toInt();
    }

    QJsonObject playerMap = json["playerMap"].toObject();
    foreach(item, playerMap.keys()) {
        QString tmp = playerMap[item].toString().mid(1);
        tmp.chop(1);
        a = tmp.section(',', 0, 0);
        b = tmp.section(',', 1, 1);
        Location l = std::make_pair(a.toInt(), b.toInt());
        m_playerMap[item] = l;
    }

    QJsonObject playerTreasureCount = json["playerTreasureCount"].toObject();
    foreach(item, playerTreasureCount.keys()) {
        m_playerTreasureCount[item] = playerTreasureCount[item].toInt();
    }
}

void GameState::write(QJsonObject &json) const
{
    json.empty();

    json["Size"] = m_Size;
    json["TreasureCount"] = m_TreasureCount;
    json["playerNumber"] = m_playerNumber;
    json["isFinish"] = m_isFinish;

    QJsonObject treasureMap;
    std::pair<Location, int> tm;
    foreach(tm, m_TreasureMap) {
        treasureMap['('+QString::number(tm.first.first)+','+QString::number(tm.first.second)+')'] = tm.second;
    }
    json["TreasureMap"] = treasureMap;

    QJsonObject playerTreasureCount;
    std::pair<PlayerID, int> ptc;
    foreach(ptc, m_playerTreasureCount) {
        playerTreasureCount[ptc.first] = ptc.second;
    }
    json["playerTreasureCount"] = playerTreasureCount;

    QJsonObject playerMap;
    std::pair<PlayerID, Location> pm;
    foreach(pm, m_playerMap) {
        playerMap[pm.first] = '('+QString::number(pm.second.first)+','+QString::number(pm.second.second)+')';
    }
    json["playerMap"] = playerMap;

}

void GameState::writeByteArray(QByteArray &barray) const
{
    QJsonObject json;
    write(json);
    QJsonDocument saveDoc(json);
    barray = saveDoc.toBinaryData();
}

void GameState::readInitByteArray(const QByteArray &barray)
{
    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(barray));
    initRead(loadDoc.object());
}

void GameState::readUpdateByteArray(const QByteArray &barray)
{
    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(barray));
    updateRead(loadDoc.object());
}

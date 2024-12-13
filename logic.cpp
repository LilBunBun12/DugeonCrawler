#include <iostream>
#include <fstream>
#include <string>
#include "logic.h"

using std::cout, std::endl, std::ifstream, std::string;
using namespace std;

/**
 * Load representation of the dungeon level from file into the 2D map.
 * Calls createMap to allocate the 2D array.
 * @param   fileName    File name of dungeon level.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object by reference to set starting position.
 * @return  pointer to 2D dynamic array representation of dungeon map with player's location., or nullptr if loading fails for any reason
 * @updates  maxRow, maxCol, player
 */
char **loadLevel(const string &fileName, int &maxRow, int &maxCol, Player &player)
{
    // cout << "Ran LoadLevel!" << endl;
    ifstream f;
    f.open(fileName);
    if (!f.is_open())
    {
        // cout << "Did not open file" << endl;
        return nullptr;
    }
    f >> maxRow >> maxCol;
    f >> player.row >> player.col;
    if (maxRow != 0 && (INT32_MAX / (maxRow * 1.0)) < maxCol )
    {
        return nullptr;
    }
    char **mat = new char *[maxRow];
    for (int i = 0; i < maxRow; i++)
    {
        mat[i] = new char[maxCol];
    }
    for (int r = 0; r < maxRow; r++)
    {
        for (int c = 0; c < maxCol; c++)
        {
            f >> mat[r][c];
        }
    }
    mat[player.row][player.col] = TILE_PLAYER;
    return mat;
}

/**
 * Translate the character direction input by the user into row or column change.
 * That is, updates the nextRow or nextCol according to the player's movement direction.
 * @param   input       Character input by the user which translates to a direction.
 * @param   nextRow     Player's next row on the dungeon map (up/down).
 * @param   nextCol     Player's next column on dungeon map (left/right).
 * @updates  nextRow, nextCol
 */
void getDirection(char input, int &nextRow, int &nextCol)
{
    // cout << "Ran getDirection!" << endl;
    switch (input)
    {
    case MOVE_UP:
        nextRow--;
        break;
    case MOVE_DOWN:
        nextRow++;
        break;
    case MOVE_LEFT:
        nextCol--;
        break;
    case MOVE_RIGHT:
        nextCol++;
        break;
    }
}

/**
 * Allocate the 2D map array.
 * Initialize each cell to TILE_OPEN.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @return  2D map array for the dungeon level, holds char type.
 */
char **createMap(int maxRow, int maxCol)
{
    // cout << "Ran createMap!" << endl;
    if (maxRow < 0 || maxCol < 0)
    {
        return nullptr;
    }
    char **mat = new char *[maxRow];
    for (int i = 0; i < maxRow; i++)
    {
        mat[i] = new char[maxCol];
    }
    for (int r = 0; r < maxRow; r++)
    {
        for (int c = 0; c < maxCol; c++)
        {
            mat[r][c] = TILE_OPEN;
        }
    }
    return mat;
}

/**
 * Deallocates the 2D map array.
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @return None
 * @update map, maxRow
 */
void deleteMap(char **&map, int &maxRow)
{
    // cout << "Ran deleteMap!" << endl;
    for (int r = 0; r < maxRow; r++)
    {
        delete[] map[r];
    }
    delete[] map;
    map = nullptr;
    maxRow = 0;
}

/**
 * Resize the 2D map by doubling both dimensions.
 * Copy the current map contents to the right, diagonal down, and below.
 * Do not duplicate the player, and remember to avoid memory leaks!
 * You can use the STATUS constants defined in logic.h to help!
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height), to be doubled.
 * @param   maxCol      Number of columns in the dungeon table (aka width), to be doubled.
 * @return  pointer to a dynamically-allocated 2D array (map) that has twice as many columns and rows in size.
 * @update maxRow, maxCol
 */
char **resizeMap(char **map, int &maxRow, int &maxCol)
{
    // cout << "Ran resizeMap!" << endl;
    if (map == nullptr || (INT32_MAX / 2.0) < maxRow || (INT32_MAX / 2.0) < maxCol)
    {
    }
    int nrl = maxRow * 2;
    int ncl = maxCol * 2;
    char **mat = new char *[nrl];
    // Creating new map
    for (int r = 0; r < nrl; r++)
    {
        mat[r] = new char[ncl];
    }
    // Section A
    for (int r = 0; r < maxRow; r++)
    {
        for (int c = 0; c < maxCol; c++)
        {
            mat[r][c] = map[r][c];
        }
    }
    // Section B
    for (int r = 0; r < maxRow; r++)
    {
        for (int c = maxCol; c < ncl; c++)
        {
            int ca = c - maxCol;
            if (map[r][ca] != TILE_PLAYER)
                mat[r][c] = map[r][ca];
            else
                mat[r][c] = TILE_OPEN;
        }
    }
    // section C
    for (int r = maxRow; r < nrl; r++)
    {
        for (int c = 0; c < maxCol; c++)
        {
            int ra = r - maxRow;
            if (map[ra][c] != TILE_PLAYER)
                mat[r][c] = map[ra][c];
            else
                mat[r][c] = TILE_OPEN;
        }
    }
    // section D
    for (int r = maxRow; r < nrl; r++)
    {
        for (int c = maxCol; c < ncl; c++)
        {
            int ra = r - maxRow;
            int ca = c - maxCol;
            if (map[ra][ca] != TILE_PLAYER)
                mat[r][c] = map[ra][ca];
            else
                mat[r][c] = TILE_OPEN;
        }
    }
    deleteMap(map, maxRow);
    maxRow = nrl;
    maxCol = ncl;
    return mat;
}

/**
 * Checks if the player can move in the specified direction and performs the move if so.
 * Cannot move out of bounds or onto TILE_PILLAR or TILE_MONSTER.
 * Cannot move onto TILE_EXIT without at least one treasure.
 * If TILE_TREASURE, increment treasure by 1.
 * Remember to update the map tile that the player moves onto and return the appropriate status.
 * You can use the STATUS constants defined in logic.h to help!
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object to by reference to see current location.
 * @param   nextRow     Player's next row on the dungeon map (up/down).
 * @param   nextCol     Player's next column on dungeon map (left/right).
 * @return  Player's movement status after updating player's position.
 * @update map contents, player
 */
int doPlayerMove(char **map, int maxRow, int maxCol, Player &player, int nextRow, int nextCol)
{
    // cout << "Ran doPlayerMove!" << endl;
    if (nextRow >= maxRow || nextRow < 0 || nextCol >= maxCol || nextCol < 0)
    {
        nextRow = player.row;
        nextCol = player.col;
        return STATUS_STAY;
    }
    char nextTile = map[nextRow][nextCol];
    if (nextTile == TILE_PILLAR || nextTile == TILE_MONSTER)
        return STATUS_STAY;
    int tempStatus = STATUS_MOVE;
    if (nextTile == TILE_DOOR)
    {
        tempStatus = STATUS_LEAVE;
    }
    if (nextTile == TILE_EXIT)
    {
        if (player.treasure == 0)
            return STATUS_STAY;
        if (player.treasure > 0)
            tempStatus = STATUS_ESCAPE;
    }
    if (nextTile == TILE_AMULET)
    {
        tempStatus = STATUS_AMULET;
    }
    if (nextTile == TILE_TREASURE)
    {
        player.treasure++;
        tempStatus = STATUS_TREASURE;
    }
    int cr = player.row;
    int cc = player.col;
    map[cr][cc] = TILE_OPEN;
    player.row = nextRow;
    player.col = nextCol;
    map[player.row][player.col] = TILE_PLAYER;
    return tempStatus;
}

/**
 * Update monster locations:
 * We check up, down, left, right from the current player position.
 * If we see an obstacle, there is no line of sight in that direction, and the monster does not move.
 * If we see a monster before an obstacle, the monster moves one tile toward the player.
 * We should update the map as the monster moves.
 * At the end, we check if a monster has moved onto the player's tile.
 * @param   map         Dungeon map.
 * @param   maxRow      Number of rows in the dungeon table (aka height).
 * @param   maxCol      Number of columns in the dungeon table (aka width).
 * @param   player      Player object by reference for current location.
 * @return  Boolean value indicating player status: true if monster reaches the player, false if not.
 * @update map contents
 */
bool doMonsterAttack(char **map, int maxRow, int maxCol, const Player &player)
{
    // cout << "Ran doMonsterAttack!" << endl;
    int dir[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int i = 0; i < 4; i++)
    {
        int r = player.row + dir[i][0];
        int c = player.col + dir[i][1];
        while (r < maxRow && r >= 0 && c < maxCol && c >= 0)
        {
            if (map[r][c] == TILE_PILLAR)
                break;
            if (map[r][c] == TILE_MONSTER)
            {
                map[r - dir[i][0]][c - dir[i][1]] = TILE_MONSTER;
                map[r][c] = TILE_OPEN;
            }
            r += dir[i][0];
            c += dir[i][1];
        }
    }
    if (map[player.row][player.col] == TILE_MONSTER)
        return true;
    return false;
}

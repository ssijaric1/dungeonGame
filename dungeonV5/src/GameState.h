#pragma once

#include <vector>
#include <utility>
#include <random>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>

class GameState {
public:
    static const int GRID_SIZE = 10;

    static const int EMPTY = 0;
    static const int PLAYER = 1;
    static const int REWARD = 2;
    static const int BANDIT = 3;
    static const int MINE = 4;
    static const int EXIT = 5;
    static const int PATH_VISUAL = 6;
    static const int EXPLORED_NODE = 7;

    using GameEventCallback = std::function<void(const std::string& event, int value)>;

    struct InitialState {
        int actualGrid[GRID_SIZE][GRID_SIZE] = { {0} };
        int playerStartX = 0;
        int playerStartY = 0;
        int exitX = 0;
        int exitY = 0;
        std::vector<std::pair<int, int>> rewards;
        std::vector<std::pair<int, int>> bandits;
        std::vector<std::pair<int, int>> mines;
    };

    GameState(std::mt19937& rng) {
        initializeGame(rng);
    }

    const int (*getDisplayGrid() const)[GRID_SIZE] { return displayGrid; }
    const InitialState& getInitialState() const { return initialState; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    int getGold() const { return gold; }
    int getCollectedRewards() const { return collectedRewards; }
    bool isGameOver() const { return gameOver; }
    bool isGameWon() const { return gameWon; }
    bool hasMetRewardRequirement() const { return gold >= 20; }
    bool hasEverReachedExit() const { return hasReachedExitOnce; }

    void setGameEventCallback(const GameEventCallback& callback) {
        gameEventCallback = callback;
    }

    void setExploredNodes(const std::vector<std::pair<int, int>>& nodes) {
        exploredNodes = nodes;
    }

    void clearExploredNodes() {
        exploredNodes.clear();
    }

    int getDisplayCell(int x, int y) const {
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
            return displayGrid[x][y];
        return EMPTY;
    }

    void applyMinePenalty() {
        gold = std::max(0, gold - 5);
    }

    bool movePlayer(int newX, int newY) {
        if (gameOver || newX < 0 || newX >= GRID_SIZE || newY < 0 || newY >= GRID_SIZE)
            return false;

        int cellType = actualGrid[newX][newY];

        actualGrid[playerX][playerY] = EMPTY;
        displayGrid[playerX][playerY] = EMPTY;

        playerX = newX;
        playerY = newY;

        if (cellType == REWARD) {
            gold += 10;
            collectedRewards++;
            actualGrid[newX][newY] = PLAYER;
            displayGrid[newX][newY] = REWARD;
            if (gameEventCallback) gameEventCallback("reward", 10);
        }
        else if (cellType == BANDIT) {
            gold = gold / 2;
            actualGrid[newX][newY] = PLAYER;
            displayGrid[newX][newY] = BANDIT;
            if (gameEventCallback) gameEventCallback("bandit", 0);
        }
        else if (cellType == MINE) {
            actualGrid[newX][newY] = PLAYER;
            displayGrid[newX][newY] = MINE;
            if (gameEventCallback) gameEventCallback("mine", 5);
        }
        else if (cellType == EXIT) {
            actualGrid[newX][newY] = PLAYER;
            displayGrid[newX][newY] = PLAYER;
            gameOver = true;
            gameWon = true;
            hasReachedExitOnce = true;
            if (gold >= 20) {
                revealAll();
                if (gameEventCallback) gameEventCallback("exit", 0);
            }
            else {
                if (gameEventCallback) gameEventCallback("exit_insufficient", gold);
            }
        }
        else {
            actualGrid[newX][newY] = PLAYER;
            displayGrid[newX][newY] = PLAYER;
        }

        return true;
    }

    void revealAll() {
        for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++)
                displayGrid[i][j] = actualGrid[i][j];
        displayGrid[playerX][playerY] = PLAYER;
    }

    void resetPlayerPosition() {
        memcpy(actualGrid, initialState.actualGrid, sizeof(actualGrid));
        memset(displayGrid, 0, sizeof(displayGrid));

        playerX = initialState.playerStartX;
        playerY = initialState.playerStartY;
        actualGrid[playerX][playerY] = PLAYER;
        displayGrid[playerX][playerY] = PLAYER;
        displayGrid[initialState.exitX][initialState.exitY] = EXIT;

        gold = 0;
        collectedRewards = 0;
        gameOver = false;
        gameWon = false;
    }

    void visualizePath(const std::vector<std::pair<int, int>>& path) {
        memset(displayGrid, 0, sizeof(displayGrid));
        displayGrid[initialState.playerStartX][initialState.playerStartY] = PLAYER;
        displayGrid[initialState.exitX][initialState.exitY] = EXIT;

        for (auto& pos : exploredNodes) {
            int x = pos.first, y = pos.second;
            if (x == initialState.playerStartX && y == initialState.playerStartY) continue;
            if (x == initialState.exitX && y == initialState.exitY) continue;
            int cell = initialState.actualGrid[x][y];
            if (cell < REWARD || cell > MINE)
                displayGrid[x][y] = EXPLORED_NODE;
        }

        for (auto& pos : path) {
            int x = pos.first, y = pos.second;
            if (x == initialState.playerStartX && y == initialState.playerStartY) continue;
            if (x == initialState.exitX && y == initialState.exitY) continue;
            displayGrid[x][y] = PATH_VISUAL;
        }

        for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++) {
                int cell = initialState.actualGrid[i][j];
                if (cell >= REWARD && cell <= MINE)
                    displayGrid[i][j] = cell;
            }
    }

    void resetVisualization() {
        exploredNodes.clear();
        if (gameOver) {
            revealAll();
        }
        else {
            for (int i = 0; i < GRID_SIZE; i++)
                for (int j = 0; j < GRID_SIZE; j++)
                    displayGrid[i][j] = actualGrid[i][j];
        }
    }

private:
    void initializeGame(std::mt19937& rng) {
        memset(actualGrid, 0, sizeof(actualGrid));
        memset(displayGrid, 0, sizeof(displayGrid));
        exploredNodes.clear();

        gold = 0;
        collectedRewards = 0;
        gameOver = false;
        gameWon = false;
        hasReachedExitOnce = false;

        initialState.rewards.clear();
        initialState.bandits.clear();
        initialState.mines.clear();

        std::uniform_int_distribution<int> rowDist(0, GRID_SIZE - 1);
        std::uniform_int_distribution<int> playerRowDist(0, GRID_SIZE - 1);

        playerX = 0;
        playerY = playerRowDist(rng);
        actualGrid[playerX][playerY] = PLAYER;
        displayGrid[playerX][playerY] = PLAYER;
        initialState.playerStartX = playerX;
        initialState.playerStartY = playerY;

        int exitRow = rowDist(rng);
        actualGrid[GRID_SIZE - 1][exitRow] = EXIT;
        displayGrid[GRID_SIZE - 1][exitRow] = EXIT;
        initialState.exitX = GRID_SIZE - 1;
        initialState.exitY = exitRow;

        for (int i = 0; i < 5; i++) placeRandomTile(rng, actualGrid, REWARD, &initialState.rewards);
        for (int i = 0; i < 5; i++) placeRandomTile(rng, actualGrid, BANDIT, &initialState.bandits);
        for (int i = 0; i < 5; i++) placeRandomTile(rng, actualGrid, MINE, &initialState.mines);

        memcpy(initialState.actualGrid, actualGrid, sizeof(actualGrid));
    }

    static void placeRandomTile(std::mt19937& rng, int grid[GRID_SIZE][GRID_SIZE], int tileType,
        std::vector<std::pair<int, int>>* positions = nullptr)
    {
        std::uniform_int_distribution<int> colDist(1, GRID_SIZE - 2);
        std::uniform_int_distribution<int> rowDist(0, GRID_SIZE - 1);

        int x, y, attempts = 0;
        do {
            x = colDist(rng);
            y = rowDist(rng);
            attempts++;
        } while (grid[x][y] != 0 && attempts < 100);

        if (grid[x][y] == 0) {
            grid[x][y] = tileType;
            if (positions) positions->push_back({ x, y });
        }
    }

    int playerX = 0, playerY = 0;
    int gold = 0;
    int collectedRewards = 0;
    bool gameOver = false, gameWon = false, hasReachedExitOnce = false;

    int actualGrid[GRID_SIZE][GRID_SIZE];
    int displayGrid[GRID_SIZE][GRID_SIZE];
    InitialState initialState;

    std::vector<std::pair<int, int>> exploredNodes;
    GameEventCallback gameEventCallback;
};
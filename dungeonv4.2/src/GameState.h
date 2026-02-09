// GameState.h
#pragma once
#include <vector>
#include <utility>
#include <random>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>  // Added for callback

class GameState {
public:
    static const int GRID_SIZE = 10;
    
    // Cell types - using same values as original
    static const int EMPTY = 0;
    static const int PLAYER = 1;
    static const int REWARD = 2;
    static const int BANDIT = 3;
    static const int MINE = 4;
    static const int EXIT = 5;
    static const int PATH_VISUAL = 6;
    static const int EXPLORED_NODE = 7;  // NEW: For explored nodes
    
    // Callback type for game events
    using GameEventCallback = std::function<void(const std::string& event, int value)>;
    
    struct InitialState {
        int actualGrid[GRID_SIZE][GRID_SIZE] = {{0}};
        int playerStartX = 0;
        int playerStartY = 0;
        int exitX = 0;
        int exitY = 0;
        std::vector<std::pair<int, int>> rewards;
        std::vector<std::pair<int, int>> bandits;
        std::vector<std::pair<int, int>> mines;
    };
    
    // Constructor
    GameState(std::mt19937& rng) {
        initializeGame(rng);
    }
    
    // Getters
    const int (*getDisplayGrid() const)[GRID_SIZE] { return displayGrid; }
    const InitialState& getInitialState() const { return initialState; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    int getGold() const { return gold; }
    bool isGameOver() const { return gameOver; }
    bool isGameWon() const { return gameWon; }
    
    // Set callback for game events
    void setGameEventCallback(const GameEventCallback& callback) {
        gameEventCallback = callback;
    }
    
    // NEW: Set explored nodes for visualization
    void setExploredNodes(const std::vector<std::pair<int, int>>& nodes) {
        exploredNodes = nodes;
    }
    
    // NEW: Clear explored nodes
    void clearExploredNodes() {
        exploredNodes.clear();
    }
    
    // For display grid access
    int getDisplayCell(int x, int y) const { 
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
            return displayGrid[x][y];
        return EMPTY;
    }
    
    // Game actions
    bool movePlayer(int newX, int newY) {
        if (gameOver || newX < 0 || newX >= GRID_SIZE || newY < 0 || newY >= GRID_SIZE)
            return false;
        
        int cellType = actualGrid[newX][newY];
        
        // Clear old position
        actualGrid[playerX][playerY] = EMPTY;
        displayGrid[playerX][playerY] = EMPTY;
        
        playerX = newX;
        playerY = newY;
        
        switch (cellType) {
            case REWARD:
                gold += 10;
                actualGrid[newX][newY] = PLAYER;
                displayGrid[newX][newY] = REWARD;  // Reveal it was a reward
                if (gameEventCallback) {
                    gameEventCallback("reward", 10);
                }
                break;
                
            case BANDIT:
                gold = gold / 2;
                actualGrid[newX][newY] = PLAYER;
                displayGrid[newX][newY] = BANDIT;  // Reveal bandit
                if (gameEventCallback) {
                    gameEventCallback("bandit", 0);
                }
                break;
                
            case MINE:
                gold = std::max(0, gold - 5);
                actualGrid[newX][newY] = PLAYER;
                displayGrid[newX][newY] = MINE;  // Reveal mine
                if (gameEventCallback) {
                    gameEventCallback("mine", 5);  // 5 gold lost
                }
                break;
                
            case EXIT:
                actualGrid[newX][newY] = PLAYER;
                displayGrid[newX][newY] = PLAYER;
                gameOver = true;
                gameWon = true;
                revealAll();
                if (gameEventCallback) {
                    gameEventCallback("exit", 0);
                }
                break;
                
            default:  // EMPTY or PLAYER
                actualGrid[newX][newY] = PLAYER;
                displayGrid[newX][newY] = PLAYER;
                break;
        }
        
        return true;
    }
    
    void revealAll() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                displayGrid[i][j] = actualGrid[i][j];
            }
        }
        // Make sure player is shown
        displayGrid[playerX][playerY] = PLAYER;
    }
    
    // Algorithm visualization
    void visualizePath(const std::vector<std::pair<int, int>>& path) {
        // Reset display grid to initial state
        memset(displayGrid, 0, sizeof(displayGrid));
        displayGrid[initialState.playerStartX][initialState.playerStartY] = PLAYER;
        displayGrid[initialState.exitX][initialState.exitY] = EXIT;
        
        // Mark explored nodes first (drawn underneath everything)
        for (const auto& pos : exploredNodes) {
            int x = pos.first;
            int y = pos.second;
            // Don't overwrite start, exit, or obstacles
            if (!(x == initialState.playerStartX && y == initialState.playerStartY) &&
                !(x == initialState.exitX && y == initialState.exitY)) {
                int cellType = initialState.actualGrid[x][y];
                if (cellType < REWARD || cellType > MINE) {
                    displayGrid[x][y] = EXPLORED_NODE;
                }
            }
        }
        
        // Mark path cells (drawn on top of explored nodes)
        for (const auto& pos : path) {
            int x = pos.first;
            int y = pos.second;
            if (!(x == initialState.playerStartX && y == initialState.playerStartY) &&
                !(x == initialState.exitX && y == initialState.exitY)) {
                displayGrid[x][y] = PATH_VISUAL;
            }
        }
        
        // Show obstacles from initial state (drawn on top of both)
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                int cellType = initialState.actualGrid[i][j];
                if (cellType >= REWARD && cellType <= MINE) {
                    displayGrid[i][j] = cellType;
                }
            }
        }
    }
    
    void resetVisualization() {
        // Reset to current game state
        exploredNodes.clear();
        if (gameOver) {
            revealAll();
        } else {
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    displayGrid[i][j] = actualGrid[i][j];
                }
            }
        }
    }
    
private:
    void initializeGame(std::mt19937& rng) {
        // Initialize grids
        memset(actualGrid, 0, sizeof(actualGrid));
        memset(displayGrid, 0, sizeof(displayGrid));
        exploredNodes.clear();
        
        // Clear initial state vectors
        initialState.rewards.clear();
        initialState.bandits.clear();
        initialState.mines.clear();
        
        // Setup random number distributions
        std::uniform_int_distribution<int> rowDist(0, GRID_SIZE - 1);
        std::uniform_int_distribution<int> colDist(1, GRID_SIZE - 2);
        std::uniform_int_distribution<int> playerRowDist(0, GRID_SIZE - 1);
        
        // Place player (always column 0)
        playerX = 0;
        playerY = playerRowDist(rng);
        actualGrid[playerX][playerY] = PLAYER;
        displayGrid[playerX][playerY] = PLAYER;
        
        // Save initial player position
        initialState.playerStartX = playerX;
        initialState.playerStartY = playerY;
        
        // Place exit (always column 9)
        int exitRow = rowDist(rng);
        actualGrid[GRID_SIZE - 1][exitRow] = EXIT;
        displayGrid[GRID_SIZE - 1][exitRow] = EXIT;
        
        // Save exit position
        initialState.exitX = GRID_SIZE - 1;
        initialState.exitY = exitRow;
        
        // Place rewards (save positions)
        for (int i = 0; i < 2; i++) {
            placeRandomTile(rng, actualGrid, REWARD, &initialState.rewards);
        }
        
        // Place bandits (save positions)
        placeRandomTile(rng, actualGrid, BANDIT, &initialState.bandits);
        
        // Place mines (save positions)
        for (int i = 0; i < 2; i++) {
            placeRandomTile(rng, actualGrid, MINE, &initialState.mines);
        }
        
        // Save the complete initial grid
        memcpy(initialState.actualGrid, actualGrid, sizeof(actualGrid));
    }
    
    static void placeRandomTile(std::mt19937& rng, int grid[GRID_SIZE][GRID_SIZE], int tileType,
                               std::vector<std::pair<int, int>>* positions = nullptr) {
        std::uniform_int_distribution<int> colDist(1, GRID_SIZE - 2);
        std::uniform_int_distribution<int> rowDist(0, GRID_SIZE - 1);
        
        int x, y;
        int attempts = 0;
        const int maxAttempts = 100;
        
        do {
            x = colDist(rng);
            y = rowDist(rng);
            attempts++;
        } while (grid[x][y] != 0 && attempts < maxAttempts);
        
        if (grid[x][y] == 0) {
            grid[x][y] = tileType;
            if (positions) {
                positions->push_back({x, y});
            }
        }
    }

    void adjustGold(int amount) {
        gold = std::max(0, gold + amount);
        // Ensure gold doesn't go negative
        if (gold < 0) gold = 0;
    }
    
    // State
    int playerX = 0, playerY = 0;
    int gold = 0;
    bool gameOver = false;
    bool gameWon = false;
    
    int actualGrid[GRID_SIZE][GRID_SIZE];
    int displayGrid[GRID_SIZE][GRID_SIZE];
    InitialState initialState;
    
    // NEW: Track explored nodes for algorithm visualization
    std::vector<std::pair<int, int>> exploredNodes;
    
    // Callback for game events
    GameEventCallback gameEventCallback;
};
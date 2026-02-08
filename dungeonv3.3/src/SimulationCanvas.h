#pragma once
#include <vector>   
#include <utility>     
#include <cstring>  
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include "Algorithms.h"
#include <gui/Image.h>
#include <algorithm>    // For std::min
#include <random>       // For random number generation
#include <ctime>        // For seeding random
#include <iostream>     // For debugging

class SimulationCanvas : public gui::Canvas {
private:
    // Drawing areas - INITIALIZED HERE
    gui::CoordType leftZoneLeft = 0;
    gui::CoordType leftZoneTop = 0;
    gui::CoordType leftZoneWidth = 0;
    gui::CoordType leftZoneHeight = 0;
    gui::CoordType rightZoneLeft = 0;
    gui::CoordType rightZoneTop = 0;
    gui::CoordType rightZoneWidth = 0;
    gui::CoordType rightZoneHeight = 0;
    
    // Random number generator
    std::mt19937 rng;
    
    // Game images
    gui::Image imgPlayer;
    gui::Image imgReward;
    gui::Image imgBandit;
    gui::Image imgMine;
    gui::Image imgExit;
    gui::Image imgBackground;  // NEW: Background image
    gui::Image imgPath;        // Optional: Image for path visualization
    
    bool imagesLoaded = true;
    bool backgroundLoaded = true;  // NEW: Track if background loaded

    // Algorithm execution state
    bool algorithmRunning = false;
    std::vector<std::pair<int, int>> algorithmPath;  // Path found by algorithm
    int currentAlgorithm = 0;  // 0=none, 1=BFS, 2=DFS, 3=Dijkstra, 4=A*, 5=Greedy
    
    // Store algorithm button areas for click detection
    gui::Rect bfsButtonRect;
    gui::Rect dfsButtonRect;
    // gui::Rect aStarrButtonRect;
    
    struct InitialGameState {
    int actualGrid[10][10] = {{0}};  // Complete original grid
    int playerStartX = 0;
    int playerStartY = 0;
    int exitX = 0;
    int exitY = 0;
    std::vector<std::pair<int, int>> rewards;    // Positions of all rewards
    std::vector<std::pair<int, int>> bandits;    // Positions of all bandits
    std::vector<std::pair<int, int>> mines;      // Positions of all mines
    };

    // Game state
    struct GameState {
    // Current game state
    int playerX = 0;
    int playerY = 0;
    int gold = 0;
    bool gameOver = false;
    bool gameWon = false;
    
    // Two-grid system for display
    int actualGrid[10][10] = {{0}};
    int displayGrid[10][10] = {{0}};
    
    // Store initial state for algorithms
    InitialGameState initialState;  // ← ADD THIS
    
    GameState(std::mt19937& rng) : playerX(0), playerY(0), gold(0), gameOver(false), gameWon(false) {
        // Initialize grids
        memset(actualGrid, 0, sizeof(actualGrid));
        memset(displayGrid, 0, sizeof(displayGrid));
        
        // Initialize initialState vectors
        initialState.rewards.clear();
        initialState.bandits.clear();
        initialState.mines.clear();
        
        // Setup random number distributions
        std::uniform_int_distribution<int> rowDist(0, 9);
        std::uniform_int_distribution<int> colDist(1, 8);
        std::uniform_int_distribution<int> playerRowDist(0, 9);
        
        // Place player (always column 0)
        playerX = 0;
        playerY = playerRowDist(rng);
        actualGrid[playerX][playerY] = 1;
        displayGrid[playerX][playerY] = 1;
        
        // Save initial player position
        initialState.playerStartX = playerX;
        initialState.playerStartY = playerY;
        
        // Place exit (always column 9)
        int exitRow = rowDist(rng);
        actualGrid[9][exitRow] = 5;
        displayGrid[9][exitRow] = 5;
        
        // Save exit position
        initialState.exitX = 9;
        initialState.exitY = exitRow;
        
        // Place rewards (save positions)
        for (int i = 0; i < 2; i++) {
            placeRandomTile(rng, actualGrid, 2, &initialState.rewards);
        }
        
        // Place bandits (save positions)
        placeRandomTile(rng, actualGrid, 3, &initialState.bandits);
        
        // Place mines (save positions)
        for (int i = 0; i < 2; i++) {
            placeRandomTile(rng, actualGrid, 4, &initialState.mines);
        }
        
        // Save the complete initial grid
        memcpy(initialState.actualGrid, actualGrid, sizeof(actualGrid));
    }
    
    // Modified placeRandomTile to save positions
    static void placeRandomTile(std::mt19937& rng, int grid[10][10], int tileType,
                               std::vector<std::pair<int, int>>* positions = nullptr) {
        std::uniform_int_distribution<int> colDist(1, 8);
        std::uniform_int_distribution<int> rowDist(0, 9);
        
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
    
    // Get initial state
    const InitialGameState& getInitialState() const {
        return initialState;
    }
    
    // At game end, reveal everything from actualGrid to displayGrid
    void revealAll() {
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                // Copy from actual grid to display grid
                displayGrid[i][j] = actualGrid[i][j];
            }
        }
        // Make sure player is still shown (in case they moved on top of something)
        displayGrid[playerX][playerY] = 1;
    }
    
    bool movePlayer(int newX, int newY) {
        if (gameOver || newX < 0 || newX >= 10 || newY < 0 || newY >= 10)
            return false;
        
        // Check what's in the ACTUAL new cell
        int cellType = actualGrid[newX][newY];
        
        // Clear old position from actual grid
        actualGrid[playerX][playerY] = 0;
        // Clear old position from display grid
        displayGrid[playerX][playerY] = 0;
        
        // Update player position
        playerX = newX;
        playerY = newY;
        
        // Handle cell effects AND REVEAL them in display grid
        if (cellType == 2) {  // Reward
            gold += 10;
            // Remove reward from actual grid (collected)
            actualGrid[newX][newY] = 1;  // Player now here
            // Show it was a reward in display grid
            displayGrid[newX][newY] = 2;  // Reveal it was a reward
            return true;
        }
        else if (cellType == 3) {  // Bandit
            gold = gold / 2;
            actualGrid[newX][newY] = 1;
            displayGrid[newX][newY] = 3;  // Reveal bandit
            return true;
        }
        else if (cellType == 4) {  // Mine
            gold = std::max(0, gold - 5);
            actualGrid[newX][newY] = 1;
            displayGrid[newX][newY] = 4;  // Reveal mine
            return true;
        }
        else if (cellType == 5) {  // Exit
            actualGrid[newX][newY] = 1;
            displayGrid[newX][newY] = 1;  // Player on exit
            gameOver = true;
            gameWon = true;
            revealAll();  // ← REVEAL EVERYTHING when game ends
            return true;
        }
        else {  // Empty cell
            actualGrid[newX][newY] = 1;
            displayGrid[newX][newY] = 1;  // Just player
            return true;
        }
    }
};


GameState gameState;
    
public:
    SimulationCanvas() 
    : gui::Canvas({ gui::InputDevice::Event::Keyboard }) // dan ga jebo
    , rng(std::random_device{}())
    , gameState(rng) 
    , imgPlayer(":player")
    , imgReward(":reward")
    , imgBandit(":bandit")
    , imgMine(":mine")
    , imgExit(":exit")
    , imgBackground(":background")
    , imgPath(":path")  // Optional: Add a path image resource
    {
        enableResizeEvent(true);
    }
    
    ~SimulationCanvas() = default;
    
    // Check if game is over
    bool isGameOver() const {
        return gameState.gameOver;
    }
    
    // Check if player won
    bool isGameWon() const {
        return gameState.gameWon;
    }
    
    // Get current gold amount
    int getGold() const {
        return gameState.gold;
    }
    
    // Get player position
    std::pair<int, int> getPlayerPosition() const {
        return {gameState.playerX, gameState.playerY};
    }

    // Algorithm methods
    void runBFSAlgorithm() {
        if (!gameState.gameOver) return;  // Only run when game is over
        
        algorithmRunning = true;
        currentAlgorithm = 1;  // BFS
        
        // Get initial state for algorithm
        const InitialGameState& initialState = gameState.getInitialState();
        
        // Call BFS algorithm
        algorithmPath = bfsSearch(
            initialState.actualGrid, 
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        // Visualize the path
        visualizeAlgorithmPath();
        reDraw();
    }
    
    void runDFSAlgorithm() {
        if (!gameState.gameOver) return;
        
        algorithmRunning = true;
        currentAlgorithm = 2;  // DFS
        
        const InitialGameState& initialState = gameState.getInitialState();
        algorithmPath = dfsSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        visualizeAlgorithmPath();
        reDraw();
    }
    
    void runDijkstraAlgorithm() {
        if (!gameState.gameOver) return;
        
        algorithmRunning = true;
        currentAlgorithm = 3;  // Dijkstra
        
        const InitialGameState& initialState = gameState.getInitialState();
        algorithmPath = dijkstraSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        visualizeAlgorithmPath();
        reDraw();
    }
    
    void runAStarAlgorithm() {
        if (!gameState.gameOver) return;
        
        algorithmRunning = true;
        currentAlgorithm = 4;  // A*
        
        const InitialGameState& initialState = gameState.getInitialState();
        algorithmPath = aStarSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        visualizeAlgorithmPath();
        reDraw();
    }
    
    void runGreedyAlgorithm() {
        if (!gameState.gameOver) return;
        
        algorithmRunning = true;
        currentAlgorithm = 5;  // Greedy
        
        const InitialGameState& initialState = gameState.getInitialState();
        algorithmPath = greedySearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        visualizeAlgorithmPath();
        reDraw();
    }
    
protected:
    // Method to visualize algorithm path on the display grid
    void visualizeAlgorithmPath() {
        // First, reset display grid to initial state (only player and exit visible)
        memset(gameState.displayGrid, 0, sizeof(gameState.displayGrid));
        gameState.displayGrid[gameState.initialState.playerStartX][gameState.initialState.playerStartY] = 1;
        gameState.displayGrid[gameState.initialState.exitX][gameState.initialState.exitY] = 5;
        
        // Mark path cells (use value 6 for path visualization)
        for (const auto& pos : algorithmPath) {
            int x = pos.first;
            int y = pos.second;
            // Only mark if not player start or exit
            if (!(x == gameState.initialState.playerStartX && y == gameState.initialState.playerStartY) &&
                !(x == gameState.initialState.exitX && y == gameState.initialState.exitY)) {
                gameState.displayGrid[x][y] = 6;  // Path visualization
            }
        }
        
        // Also show obstacles from initial state
        const auto& initialState = gameState.getInitialState();
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int cellType = initialState.actualGrid[i][j];
                // Show rewards, bandits, mines (2, 3, 4)
                if (cellType >= 2 && cellType <= 4) {
                    gameState.displayGrid[i][j] = cellType;
                }
            }
        }
    }

    // Method to reset algorithm visualization
    void resetAlgorithmVisualization() {
        algorithmRunning = false;
        currentAlgorithm = 0;
        algorithmPath.clear();
        
        // Reset display grid to current game state
        if (gameState.gameOver) {
            gameState.revealAll();  // Show everything if game is over
        } else {
            // Otherwise show normal game state
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    gameState.displayGrid[i][j] = gameState.actualGrid[i][j];
                }
            }
        }
        reDraw();
    }

    // Enhanced key handler with algorithm shortcuts
    bool onKeyPressed(const gui::Key& key) override
    {
        if (key.isVirtual())
        {
            gui::Key::Virtual chVirt = key.getVirtual();
            
            if (chVirt == gui::Key::Virtual::Right)
            {
                gameState.movePlayer(gameState.playerX + 1, gameState.playerY);
                reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Left)
            {
                gameState.movePlayer(gameState.playerX - 1, gameState.playerY);
                reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Up)
            {
                gameState.movePlayer(gameState.playerX, gameState.playerY - 1);
                reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Down)
            {
                gameState.movePlayer(gameState.playerX, gameState.playerY + 1);
                reDraw();
                return true;
            }
        }
        
        // Add algorithm triggers (only when game is over)
        if (key.isASCII() && gameState.gameOver) {
            char ch = key.getChar();
            switch(ch) {
                case '1':
                case 'b':
                case 'B':
                    runBFSAlgorithm();
                    return true;
                case '2':
                case 'd':
                case 'D':
                    runDFSAlgorithm();
                    return true;
                case '3':
                case 'j':
                case 'J':  // Dijkstra
                    runDijkstraAlgorithm();
                    return true;
                case '4':
                case 'a':
                case 'A':  // A*
                    runAStarAlgorithm();
                    return true;
                case '5':
                case 'g':
                case 'G':  // Greedy
                    runGreedyAlgorithm();
                    return true;
                case 'r':
                case 'R':  // Reset algorithm visualization
                    resetAlgorithmVisualization();
                    return true;
            }
        }
        
        return gui::Canvas::onKeyPressed(key);
    }

    void onResize(const gui::Size& newSize) override {
        // Determine which dimension is smaller
        gui::CoordType minDimension = std::min(newSize.width, newSize.height);
        
        // Make left square 70% of the smaller dimension
        leftZoneWidth = minDimension * 0.9;
        leftZoneHeight = leftZoneWidth;  // Perfect 1:1
        
        // Right zone: wider (20% of width) and 90% of height
        rightZoneWidth = newSize.width * 0.20;  // Increased to 20% for wider buttons area
        rightZoneHeight = newSize.height * 0.9;
        
        // Reduce margins between zones - bring them closer together
        // Only 2% total margin (1% on each side of the gap)
        gui::CoordType zoneGap = newSize.width * 0.02;  // 1% gap between zones
        
        // Left zone: position with 5% margin from left edge
        leftZoneLeft = newSize.width * 0.05;
        
        // Center left zone vertically (5% top/bottom margin)
        leftZoneTop = (newSize.height - leftZoneHeight) / 2;
        
        // Right zone: position right after the left zone with a small gap
        rightZoneLeft = leftZoneLeft + leftZoneWidth + zoneGap;
        
        // Center right zone vertically (5% top/bottom margin)
        rightZoneTop = (newSize.height - rightZoneHeight) / 2;
        
        this->reDraw();
    }

    void onDraw(const gui::Rect& rect) override {
        // Draw background
        gui::Shape bg;
        bg.createRect(rect);
        bg.drawFill(td::ColorID::Black);
        
        // Draw left zone (1:1 square)
        drawZone(leftZoneLeft, leftZoneTop, leftZoneWidth, leftZoneHeight, 
                 "Game grid", td::ColorID::White);
        
        // Draw right zone (wider for buttons)  
        drawZone(rightZoneLeft, rightZoneTop, rightZoneWidth, rightZoneHeight,
                 "Controls", td::ColorID::White);
        
        // Draw game grid with all elements
        drawGameGrid();
        
        // Draw game status in right zone
        drawGameStatus();
        
        // Draw algorithm status if running
        if (algorithmRunning) {
            drawAlgorithmStatus();
        }
    }

private:
    void drawZone(gui::CoordType left, gui::CoordType top, 
                  gui::CoordType width, gui::CoordType height,
                  const std::string& label, td::ColorID borderColor) {
        // Draw zone border
        gui::Shape zoneBorder;
        zoneBorder.createRect(gui::Rect(left, top, left + width, top + height));
        zoneBorder.drawWire(borderColor);
        
        // Draw zone label above the box
        gui::DrawableString::draw(label.c_str(), label.length(),
                                 gui::Rect(left, top - 35, left + width, top - 5),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::LightGray,
                                 td::TextAlignment::Center,
                                 td::VAlignment::Center);
    }
    
    void drawGameGrid() {
        const int GRID_SIZE = 10;
        
        // Calculate grid area with margin inside left zone
        gui::CoordType margin = leftZoneWidth * 0.01;
        gui::CoordType gridAreaSize = leftZoneWidth - 2 * margin;
        gui::CoordType cellSize = gridAreaSize / GRID_SIZE;
        
        // Calculate starting position for grid
        gui::CoordType gridStartX = leftZoneLeft + margin;
        gui::CoordType gridStartY = leftZoneTop + margin;
        
        // Draw grid background - either image or black
        if (backgroundLoaded) {
            // Draw background image instead of black rectangle
            imgBackground.draw(gui::Rect(gridStartX, gridStartY, 
                                        gridStartX + gridAreaSize, 
                                        gridStartY + gridAreaSize));
        } else {
            // Fall back to black background
            gui::Shape gridBg;
            gridBg.createRect(gui::Rect(gridStartX, gridStartY, 
                                       gridStartX + gridAreaSize, 
                                       gridStartY + gridAreaSize));
            gridBg.drawFill(td::ColorID::Black);
        }
        
        // Draw horizontal grid lines
        for (int j = 0; j <= GRID_SIZE; j++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = {gridStartX, gridStartY + j * cellSize};
            points[1] = {gridStartX + gridAreaSize, gridStartY + j * cellSize};
            line.createLines(points, 2, 5);
            line.drawWire(td::ColorID::Black);
        }
        
        // Draw vertical grid lines
        for (int i = 0; i <= GRID_SIZE; i++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = {gridStartX + i * cellSize, gridStartY};
            points[1] = {gridStartX + i * cellSize, gridStartY + gridAreaSize};
            line.createLines(points, 2, 5);
            line.drawWire(td::ColorID::Black);
        }
        
        // Draw game elements
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                int cellType = gameState.displayGrid[i][j]; 
                if (cellType != 0) {
                    drawCellContent(gridStartX + i * cellSize,
                                    gridStartY + j * cellSize,
                                    cellSize, cellType);
                }
            }
        }
    }
    
    void drawCellContent(gui::CoordType x, gui::CoordType y, 
                        gui::CoordType size, int cellType) {
        // Leave a small margin inside the cell
        gui::CoordType margin = size * 0.1;
        gui::Rect cellRect(x + margin, y + margin, 
                          x + size - margin, y + size - margin);
        
        // Handle algorithm path first (value 6)
        if (cellType == 6) {
            // Draw path visualization - purple for algorithm path
            gui::Shape pathShape;
            pathShape.createRect(cellRect);
            pathShape.drawFill(td::ColorID::Purple);
            
            // Optionally add a path image if available
            try {
                imgPath.draw(cellRect);
            } catch (...) {
                // No path image, just use colored square
            }
            return;
        }
        
        if (imagesLoaded) {
            // Try to draw image
            try {
                switch(cellType) {
                    case 1:  // Player
                        imgPlayer.draw(cellRect);
                        break;
                    case 2:  // Reward
                        imgReward.draw(cellRect);
                        break;
                    case 3:  // Bandit
                        imgBandit.draw(cellRect);
                        break;
                    case 4:  // Mine
                        imgMine.draw(cellRect);
                        break;
                    case 5:  // Exit
                        imgExit.draw(cellRect);
                        break;
                }
                return; // Success, exit early
            } catch (...) {
                // If drawing fails, fall back to colored square
            }
        }
        
        // Fall back to colored square
        gui::Shape cellShape;
        cellShape.createRect(cellRect);
        
        switch(cellType) {
            case 1:  // Player - Green
                cellShape.drawFill(td::ColorID::Green);
                break;
            case 2:  // Reward - Yellow
                cellShape.drawFill(td::ColorID::Yellow);
                break;
            case 3:  // Bandit - Blue
                cellShape.drawFill(td::ColorID::Blue);
                break;
            case 4:  // Mine - Red
                cellShape.drawFill(td::ColorID::Red);
                break;
            case 5:  // Exit - White
                cellShape.drawFill(td::ColorID::White);
                break;
        }
    }
    
    void drawGameStatus() {
        // Status area inside right zone
        gui::CoordType statusX = rightZoneLeft + 15;  // Moderate margin
        gui::CoordType statusY = rightZoneTop + 20;
        gui::CoordType lineHeight = 30;
        
        // Game title
        gui::DrawableString::draw("Dungeon Game", 12,
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 40),
                                 gui::Font::ID::SystemBold,
                                 td::ColorID::White,
                                 td::TextAlignment::Center,
                                 td::VAlignment::Center);
        
        statusY += 50;
        
        // Gold
        std::string goldStr = "Gold: " + std::to_string(gameState.gold);
        gui::DrawableString::draw(goldStr.c_str(), goldStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::Yellow);
        
        statusY += 40;
        
        // Player position
        std::string posStr = "Position: " + std::to_string(gameState.playerX) + 
                            "," + std::to_string(gameState.playerY);
        gui::DrawableString::draw(posStr.c_str(), posStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::Green);
        
        statusY += 40;
        
        // Game status
        std::string statusStr;
        td::ColorID statusColor = td::ColorID::White;
        
        if (gameState.gameOver) {
            if (gameState.gameWon) {
                statusStr = "YOU WIN!";
                statusColor = td::ColorID::Green;
            } else {
                statusStr = "GAME OVER";
                statusColor = td::ColorID::Red;
            }
        } else {
            statusStr = "Use arrows to move";
            statusColor = td::ColorID::LightGray;
        }
        
        gui::DrawableString::draw(statusStr.c_str(), statusStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 40),
                                 gui::Font::ID::SystemNormal,
                                 statusColor,
                                 td::TextAlignment::Center);
        
        // Draw legend
        statusY = rightZoneTop + 250;  // Position below game status
        
        gui::DrawableString::draw("Legend:", 7,
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemBold,
                                 td::ColorID::White);
        
        statusY += 35;
        
        // Draw legend items
        std::pair<std::string, td::ColorID> legend[] = {
        {"1 or B - BFS", td::ColorID::White},
        {"2 or D - DFS", td::ColorID::White},
        {"3 or J - Dijkstra", td::ColorID::White},
        {"4 or A - A*", td::ColorID::White},
        {"5 or G - Greedy", td::ColorID::White},
        {"R - Reset View", td::ColorID::White}
        };
        
        for (int i = 0; i < 6; i++) {
            // Draw colored square
            gui::Shape legendShape;
            legendShape.createRect(gui::Rect(statusX, statusY + 5,
                                            statusX + 18, statusY + 23));
            legendShape.drawFill(legend[i].second);
            
            // Draw text
            gui::DrawableString::draw(legend[i].first.c_str(), legend[i].first.length(),
                                     gui::Rect(statusX + 25, statusY,
                                              statusX + rightZoneWidth - 30, statusY + 28),
                                     gui::Font::ID::SystemNormal,
                                     td::ColorID::White);
            
            statusY += 35;
        }
        
        // Draw algorithm instructions
        if (gameState.gameOver) {
            statusY += 20;
            gui::DrawableString::draw("Algorithms:", 11,
                                     gui::Rect(statusX, statusY, 
                                              statusX + rightZoneWidth - 30, statusY + 30),
                                     gui::Font::ID::SystemBold,
                                     td::ColorID::LightBlue);
            
            statusY += 35;
            
            std::string algoInstructions = "Press: 1-BFS 2-DFS\n3-Dijkstra 4-A*\n5-Greedy R-Reset";
            gui::DrawableString::draw(algoInstructions.c_str(), algoInstructions.length(),
                                     gui::Rect(statusX, statusY, 
                                              statusX + rightZoneWidth - 30, statusY + 90),
                                     gui::Font::ID::SystemSmaller,
                                     td::ColorID::LightGray,
                                     td::TextAlignment::Left,
                                     td::VAlignment::Top);
        }
    }
    
    void drawAlgorithmStatus() {
        // Draw algorithm status in top-left corner of grid
        gui::CoordType statusX = leftZoneLeft + 10;
        gui::CoordType statusY = leftZoneTop + 10;
        
        std::string algoName;
        switch(currentAlgorithm) {
            case 1: algoName = "BFS"; break;
            case 2: algoName = "DFS"; break;
            case 3: algoName = "Dijkstra"; break;
            case 4: algoName = "A*"; break;
            case 5: algoName = "Greedy"; break;
            default: algoName = "Algorithm";
        }
        
        std::string statusText = algoName + " Path: " + std::to_string(algorithmPath.size()) + " steps";
        
        // Draw semi-transparent background
        gui::Shape bg;
        bg.createRect(gui::Rect(statusX - 5, statusY - 5, 
                               statusX + 150, statusY + 25));
        bg.drawFill(td::ColorID::DarkGray);
        
        // Draw text
        gui::DrawableString::draw(statusText.c_str(), statusText.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + 140, statusY + 20),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::White);
    }
};
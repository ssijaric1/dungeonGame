#pragma once
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
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
    
    bool imagesLoaded = true;
    bool backgroundLoaded = true;  // NEW: Track if background loaded
    
    // Game state
    struct GameState {
        int playerX = 0;
        int playerY = 0;
        int gold = 0;
        bool gameOver = false;
        bool gameWon = false;
        
        // Cell types: 0=empty, 1=player, 2=reward, 3=bandit, 4=mine, 5=exit
        int grid[10][10] = {{0}};  // Initialize all to 0
        
        GameState(std::mt19937& rng) : playerX(0), playerY(0), gold(0), gameOver(false), gameWon(false) {
            memset(grid, 0, sizeof(grid));
            // Setup random number distributions
            std::uniform_int_distribution<int> rowDist(0, 9);
            std::uniform_int_distribution<int> colDist(0, 9);
            std::uniform_int_distribution<int> playerRowDist(0, 9); // Player can be in any row in first column
            
            // Place player in first column (x=0), random row
            playerX = 0;  // First column
            playerY = playerRowDist(rng);
            grid[playerX][playerY] = 1;    // Player start
            
            // Place exit in last column (x=9), random row
            int exitRow = rowDist(rng);
            grid[9][exitRow] = 5;          // Exit
            
            // Place rewards (2 of them)
            placeRandomTile(rng, grid, 2, 2);  // 2 rewards
            
            // Place bandits (1 of them)
            placeRandomTile(rng, grid, 1, 3);  // 1 bandit
            
            // Place mines (2 of them)
            placeRandomTile(rng, grid, 2, 4);  // 2 mines
        }
        
        // Helper function to place random tiles
        static void placeRandomTile(std::mt19937& rng, int grid[10][10], int count, int tileType) {
            std::uniform_int_distribution<int> rowDist(0, 9);
            std::uniform_int_distribution<int> colDist(0, 9);
            
            for (int i = 0; i < count; i++) {
                int x, y;
                int attempts = 0;
                const int maxAttempts = 100;  // Prevent infinite loop
                
                // Find empty cell
                do {
                    x = colDist(rng);
                    y = rowDist(rng);
                    attempts++;
                } while (grid[x][y] != 0 && attempts < maxAttempts);
                
                // If we found an empty cell, place the tile
                if (grid[x][y] == 0) {
                    grid[x][y] = tileType;
                }
                // If we couldn't find an empty cell after maxAttempts, give up
            }
        }
        
        bool movePlayer(int newX, int newY) {
            if (gameOver || newX < 0 || newX >= 10 || newY < 0 || newY >= 10)
                return false;
            
            // Check what's in the new cell
            int cellType = grid[newX][newY];
            
            // Move player
            grid[playerX][playerY] = 0;  // Clear old position
            playerX = newX;
            playerY = newY;
            
            // Handle cell effects
            if (cellType == 2) {  // Reward
                gold += 10;
                // Reward disappears after collecting
                grid[newX][newY] = 1;  // Player moves here
                return true;
            }
            else if (cellType == 3) {  // Bandit
                gold = gold / 2;  // Lose half gold
                grid[newX][newY] = 1;  // Player moves here
                return true;
            }
            else if (cellType == 4) {  // Mine
                // For now, just lose 5 gold
                gold = std::max(0, gold - 5);
                grid[newX][newY] = 1;  // Player moves here
                return true;
            }
            else if (cellType == 5) {  // Exit
                grid[newX][newY] = 1;  // Player moves here
                gameOver = true;
                gameWon = true;
                return true;
            }
            else {  // Empty cell
                grid[newX][newY] = 1;  // Player moves here
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

protected:

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
                int cellType = gameState.grid[i][j];
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
            statusStr = "Use buttons to move";
            statusColor = td::ColorID::LightGray;
        }
        
        gui::DrawableString::draw(statusStr.c_str(), statusStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 40),
                                 gui::Font::ID::SystemNormal,
                                 statusColor,
                                 td::TextAlignment::Center);
        
        // Draw legend - moved down to make room for buttons
        statusY = rightZoneTop + 250;  // Position below where buttons will be
        
        gui::DrawableString::draw("Legend:", 7,
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemBold,
                                 td::ColorID::White);
        
        statusY += 35;
        
        // Draw legend items (use colored squares for now, could be updated to show images)
        std::pair<std::string, td::ColorID> legend[] = {
            {"Player", td::ColorID::Green},
            {"Reward", td::ColorID::Yellow},
            {"Bandit", td::ColorID::Blue},
            {"Mine", td::ColorID::Red},
            {"Exit", td::ColorID::White}
        };
        
        for (int i = 0; i < 5; i++) {
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
    }
};
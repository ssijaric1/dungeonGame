// SimulationCanvas.h
#pragma once
#include <vector>   
#include <utility>     
#include <cstring>  
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Image.h>
#include <gui/Alert.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include "Algorithms.h"
#include "GameState.h"
#include "QuestionsPopUp.h"  // Add this line

class SimulationCanvas : public gui::Canvas {
public:
    SimulationCanvas() 
    : gui::Canvas({ gui::InputDevice::Event::Keyboard })
    , rng(std::random_device{}())  // Initialize rng FIRST
    , gameState(rng)               // THEN initialize gameState with rng
    , imgPlayer(":player")
    , imgReward(":reward")
    , imgBandit(":bandit")
    , imgMine(":mine")
    , imgExit(":exit")
    , imgBackground(":background")
    , imgPath(":path") {
        enableResizeEvent(true);
        
        // Set up game event callbacks
        gameState.setGameEventCallback([this](const std::string& event, int value) {
            handleGameEvent(event, value);
        });
        
        std::cout << "SimulationCanvas initialized successfully" << std::endl;
    }
    
    ~SimulationCanvas() = default;
    
    // Game state queries
    bool isGameOver() const { return gameState.isGameOver(); }
    bool isGameWon() const { return gameState.isGameWon(); }
    int getGold() const { return gameState.getGold(); }
    std::pair<int, int> getPlayerPosition() const { 
        return {gameState.getPlayerX(), gameState.getPlayerY()}; 
    }

    // Algorithm methods
    void runBFSAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "BFS: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        currentAlgorithm = 1;  // BFS
        
        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::bfsSearch(
            initialState.actualGrid, 
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        std::cout << "BFS path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }
    
    void runDFSAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "DFS: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        currentAlgorithm = 2;  // DFS
        
        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::dfsSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        std::cout << "DFS path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }
    
    void runDijkstraAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "Dijkstra: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        currentAlgorithm = 3;  // Dijkstra
        
        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::dijkstraSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        std::cout << "Dijkstra path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }
    
    void runAStarAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "A*: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        currentAlgorithm = 4;  // A*
        
        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::aStarSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        std::cout << "A* path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }
    
    void runGreedyAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "Greedy: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        currentAlgorithm = 5;  // Greedy
        
        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::greedySearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        std::cout << "Greedy path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }
    
    void resetAlgorithmVisualization() {
        algorithmRunning = false;
        currentAlgorithm = 0;
        algorithmPath.clear();
        gameState.resetVisualization();
        std::cout << "Algorithm visualization reset" << std::endl;
        reDraw();
    }
    
protected:
    bool onKeyPressed(const gui::Key& key) override {
        if (key.isVirtual()) {
            gui::Key::Virtual chVirt = key.getVirtual();
            
            if (chVirt == gui::Key::Virtual::Right) {
                bool moved = gameState.movePlayer(gameState.getPlayerX() + 1, gameState.getPlayerY());
                if (moved) reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Left) {
                bool moved = gameState.movePlayer(gameState.getPlayerX() - 1, gameState.getPlayerY());
                if (moved) reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Up) {
                bool moved = gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() - 1);
                if (moved) reDraw();
                return true;
            }
            if (chVirt == gui::Key::Virtual::Down) {
                bool moved = gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() + 1);
                if (moved) reDraw();
                return true;
            }
        }
        
        // Algorithm triggers (only when game is over)
        if (key.isASCII()) {
            char ch = key.getChar();
            switch(ch) {
                case '1': case 'b': case 'B': 
                    runBFSAlgorithm(); 
                    return true;
                case '2': case 'd': case 'D': 
                    runDFSAlgorithm(); 
                    return true;
                case '3': case 'j': case 'J': 
                    runDijkstraAlgorithm(); 
                    return true;
                case '4': case 'a': case 'A': 
                    runAStarAlgorithm(); 
                    return true;
                case '5': case 'g': case 'G': 
                    runGreedyAlgorithm(); 
                    return true;
                case 'r': case 'R': 
                    resetAlgorithmVisualization(); 
                    return true;
                case 'n': case 'N':  // New game shortcut for testing
                    resetGame();
                    return true;
            }
        }
        
        return gui::Canvas::onKeyPressed(key);
    }
    
    void resetGame() {
        // Create new random number generator and game state
        rng = std::mt19937(std::random_device{}());
        gameState = GameState(rng);
        
        // Re-setup the callback for the new game state
        gameState.setGameEventCallback([this](const std::string& event, int value) {
            handleGameEvent(event, value);
        });
        
        algorithmRunning = false;
        currentAlgorithm = 0;
        algorithmPath.clear();
        std::cout << "New game started" << std::endl;
        reDraw();
    }
    
    // Handle game events (like mine hits) - SIMPLE VERSION
    void handleGameEvent(const std::string& event, int value) {
    if (event == "mine") {
        // Create dialog with random question using factory method
        DialogLogin* dlg = DialogLogin::createWithRandomQuestion(this);
        
        dlg->openModal([this, value](gui::Dialog* pDlg) {
            DialogLogin* questionDlg = (DialogLogin*) pDlg;
            
            if (questionDlg->isAnswerCorrect()) {
                td::String message;
                message.format("Correct! You avoid losing %d gold!", value);
                gui::Alert::show("Mine Avoided!", message);
            } else {
                td::String message;
                message.format("Wrong answer! You lose %d gold.", value);
                gui::Alert::show("Mine Hit!", message);
            }
        });
    }
        else if (event == "reward") {
            td::String message;
            message.format("You found %d gold!\nTotal gold: %d", value, gameState.getGold());
            gui::Alert::show("Reward Found!", message);
        }
        else if (event == "bandit") {
            td::String message;
            message.format("A bandit stole half your gold!\nRemaining gold: %d", gameState.getGold());
            gui::Alert::show("Bandit Attack!", message);
        }
        else if (event == "exit") {
            td::String message;
            message.format("You escaped the dungeon!\nFinal gold: %d", gameState.getGold());
            gui::Alert::show("You Win!", message);
        }
    }
    
    void onResize(const gui::Size& newSize) override {
        gui::CoordType minDimension = std::min(newSize.width, newSize.height);
        
        leftZoneWidth = minDimension * 0.9;
        leftZoneHeight = leftZoneWidth;
        
        rightZoneWidth = newSize.width * 0.20;
        rightZoneHeight = newSize.height * 0.9;
        
        gui::CoordType zoneGap = newSize.width * 0.02;
        
        leftZoneLeft = newSize.width * 0.05;
        leftZoneTop = (newSize.height - leftZoneHeight) / 2;
        
        rightZoneLeft = leftZoneLeft + leftZoneWidth + zoneGap;
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
    // Drawing methods
    void drawZone(gui::CoordType left, gui::CoordType top, 
                  gui::CoordType width, gui::CoordType height,
                  const std::string& label, td::ColorID borderColor) {
        gui::Shape zoneBorder;
        zoneBorder.createRect(gui::Rect(left, top, left + width, top + height));
        zoneBorder.drawWire(borderColor);
        
        gui::DrawableString::draw(label.c_str(), label.length(),
                                 gui::Rect(left, top - 35, left + width, top - 5),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::LightGray,
                                 td::TextAlignment::Center,
                                 td::VAlignment::Center);
    }
    
    void drawGameGrid() {
        const int GRID_SIZE = GameState::GRID_SIZE;
        
        gui::CoordType margin = leftZoneWidth * 0.01;
        gui::CoordType gridAreaSize = leftZoneWidth - 2 * margin;
        gui::CoordType cellSize = gridAreaSize / GRID_SIZE;
        
        gui::CoordType gridStartX = leftZoneLeft + margin;
        gui::CoordType gridStartY = leftZoneTop + margin;
        
        // Draw grid background
        if (backgroundLoaded) {
            try {
                imgBackground.draw(gui::Rect(gridStartX, gridStartY, 
                                            gridStartX + gridAreaSize, 
                                            gridStartY + gridAreaSize));
            } catch (...) {
                backgroundLoaded = false;
                gui::Shape gridBg;
                gridBg.createRect(gui::Rect(gridStartX, gridStartY, 
                                           gridStartX + gridAreaSize, 
                                           gridStartY + gridAreaSize));
                gridBg.drawFill(td::ColorID::Black);
            }
        } else {
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
            line.drawWire(td::ColorID::DarkGray);
        }
        
        // Draw vertical grid lines
        for (int i = 0; i <= GRID_SIZE; i++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = {gridStartX + i * cellSize, gridStartY};
            points[1] = {gridStartX + i * cellSize, gridStartY + gridAreaSize};
            line.createLines(points, 2, 5);
            line.drawWire(td::ColorID::DarkGray);
        }
        
        // Draw game elements
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                int cellType = gameState.getDisplayCell(i, j); 
                if (cellType != GameState::EMPTY) {
                    drawCellContent(gridStartX + i * cellSize,
                                    gridStartY + j * cellSize,
                                    cellSize, cellType);
                }
            }
        }
    }
    
    void drawCellContent(gui::CoordType x, gui::CoordType y, 
                        gui::CoordType size, int cellType) {
        gui::CoordType margin = size * 0.1;
        gui::Rect cellRect(x + margin, y + margin, 
                          x + size - margin, y + size - margin);
        
        // Handle algorithm path first
        if (cellType == GameState::PATH_VISUAL) {
            gui::Shape pathShape;
            pathShape.createRect(cellRect);
            pathShape.drawFill(td::ColorID::Purple);
            
            try {
                imgPath.draw(cellRect);
            } catch (...) {
                // No path image, just use colored square
            }
            return;
        }
        
        if (imagesLoaded) {
            try {
                switch(cellType) {
                    case GameState::PLAYER: 
                        imgPlayer.draw(cellRect); 
                        break;
                    case GameState::REWARD: 
                        imgReward.draw(cellRect); 
                        break;
                    case GameState::BANDIT: 
                        imgBandit.draw(cellRect); 
                        break;
                    case GameState::MINE:   
                        imgMine.draw(cellRect); 
                        break;
                    case GameState::EXIT:   
                        imgExit.draw(cellRect); 
                        break;
                }
                return;
            } catch (...) {
                // If drawing fails, fall back to colored square
                imagesLoaded = false;
            }
        }
        
        // Fall back to colored square
        gui::Shape cellShape;
        cellShape.createRect(cellRect);
        
        switch(cellType) {
            case GameState::PLAYER: 
                cellShape.drawFill(td::ColorID::Green); 
                break;
            case GameState::REWARD: 
                cellShape.drawFill(td::ColorID::Yellow); 
                break;
            case GameState::BANDIT: 
                cellShape.drawFill(td::ColorID::Blue); 
                break;
            case GameState::MINE:   
                cellShape.drawFill(td::ColorID::Red); 
                break;
            case GameState::EXIT:   
                cellShape.drawFill(td::ColorID::White); 
                break;
        }
    }
    
    void drawGameStatus() {
        gui::CoordType statusX = rightZoneLeft + 15;
        gui::CoordType statusY = rightZoneTop + 20;
        
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
        std::string goldStr = "Gold: " + std::to_string(gameState.getGold());
        gui::DrawableString::draw(goldStr.c_str(), goldStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::Yellow);
        
        statusY += 40;
        
        // Player position
        auto playerX = gameState.getPlayerX();
        auto playerY = gameState.getPlayerY();
        std::string posStr = "Position: " + std::to_string(playerX) + "," + std::to_string(playerY);
        gui::DrawableString::draw(posStr.c_str(), posStr.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::Green);
        
        statusY += 40;
        
        // Game status
        std::string statusStr;
        td::ColorID statusColor = td::ColorID::White;
        
        if (gameState.isGameOver()) {
            if (gameState.isGameWon()) {
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
        statusY = rightZoneTop + 250;
        
        gui::DrawableString::draw("Legend:", 7,
                                 gui::Rect(statusX, statusY, 
                                          statusX + rightZoneWidth - 30, statusY + 30),
                                 gui::Font::ID::SystemBold,
                                 td::ColorID::White);
        
        statusY += 35;
        
        std::pair<std::string, td::ColorID> legend[] = {
            {"1 or B - BFS", td::ColorID::White},
            {"2 or D - DFS", td::ColorID::White},
            {"3 or J - Dijkstra", td::ColorID::White},
            {"4 or A - A*", td::ColorID::White},
            {"5 or G - Greedy", td::ColorID::White},
            {"R - Reset View", td::ColorID::White},
            {"N - New Game", td::ColorID::White}
        };
        
        for (int i = 0; i < 7; i++) {
            gui::Shape legendShape;
            legendShape.createRect(gui::Rect(statusX, statusY + 5,
                                            statusX + 18, statusY + 23));
            legendShape.drawFill(legend[i].second);
            
            gui::DrawableString::draw(legend[i].first.c_str(), legend[i].first.length(),
                                     gui::Rect(statusX + 25, statusY,
                                              statusX + rightZoneWidth - 30, statusY + 28),
                                     gui::Font::ID::SystemNormal,
                                     td::ColorID::White);
            
            statusY += 35;
        }
        
        // Draw algorithm instructions
        if (gameState.isGameOver()) {
            statusY += 20;
            gui::DrawableString::draw("Algorithms:", 11,
                                     gui::Rect(statusX, statusY, 
                                              statusX + rightZoneWidth - 30, statusY + 30),
                                     gui::Font::ID::SystemBold,
                                     td::ColorID::LightBlue);
            
            statusY += 35;
            
            std::string algoInstructions = "Press:\n1-BFS\n2-DFS\n3-Dijkstra\n4-A*\n5-Greedy\nR-Reset\nN-New Game";
            gui::DrawableString::draw(algoInstructions.c_str(), algoInstructions.length(),
                                     gui::Rect(statusX, statusY, 
                                              statusX + rightZoneWidth - 30, statusY + 140),
                                     gui::Font::ID::SystemSmaller,
                                     td::ColorID::LightGray,
                                     td::TextAlignment::Left,
                                     td::VAlignment::Top);
        }
    }
    
    void drawAlgorithmStatus() {
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
        
        gui::Shape bg;
        bg.createRect(gui::Rect(statusX - 5, statusY - 5, 
                               statusX + 150, statusY + 25));
        bg.drawFill(td::ColorID::DarkGray);
        
        gui::DrawableString::draw(statusText.c_str(), statusText.length(),
                                 gui::Rect(statusX, statusY, 
                                          statusX + 140, statusY + 20),
                                 gui::Font::ID::SystemNormal,
                                 td::ColorID::White);
    }
    
private:
    // IMPORTANT: rng MUST be declared BEFORE gameState
    std::mt19937 rng;
    
    // Game state - depends on rng, so must come after
    GameState gameState;
    
    // Drawing areas
    gui::CoordType leftZoneLeft = 0;
    gui::CoordType leftZoneTop = 0;
    gui::CoordType leftZoneWidth = 0;
    gui::CoordType leftZoneHeight = 0;
    gui::CoordType rightZoneLeft = 0;
    gui::CoordType rightZoneTop = 0;
    gui::CoordType rightZoneWidth = 0;
    gui::CoordType rightZoneHeight = 0;
    
    // Game images
    gui::Image imgPlayer;
    gui::Image imgReward;
    gui::Image imgBandit;
    gui::Image imgMine;
    gui::Image imgExit;
    gui::Image imgBackground;
    gui::Image imgPath;
    
    bool imagesLoaded = true;
    bool backgroundLoaded = true;
    
    // Algorithm execution state
    bool algorithmRunning = false;
    std::vector<std::pair<int, int>> algorithmPath;
    int currentAlgorithm = 0;  // 0=none, 1=BFS, 2=DFS, 3=Dijkstra, 4=A*, 5=Greedy
};
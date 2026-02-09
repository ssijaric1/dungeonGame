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
#include <chrono>
#include "Algorithms.h"
#include "GameState.h"
#include "QuestionsPopUp.h"

class SimulationCanvas : public gui::Canvas {
public:
    SimulationCanvas() 
    : gui::Canvas({ gui::InputDevice::Event::Keyboard, gui::InputDevice::Event::PrimaryClicks })
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

    // Algorithm methods - UPDATED with animation support
 void runBFSAlgorithm() {
    if (!gameState.isGameOver()) {
        std::cout << "BFS: Game must be over to run algorithm" << std::endl;
        return;
    }
    
    algorithmRunning = true;
    isAnimating = false; // Don't start automatically
    currentAlgorithm = 1;  // BFS
    
    const auto& initialState = gameState.getInitialState();
    auto result = DungeonAlgorithms::bfsSearch(
        initialState.actualGrid, 
        {initialState.playerStartX, initialState.playerStartY},
        {initialState.exitX, initialState.exitY}
    );
    
    fullAlgorithmPath = result.path;
    fullExploredNodes = result.exploredNodes;
    
    // Initialize animation state
    currentExploredIndex = 0;
    currentPathIndex = 0;
    animationPhase = 0;  // 0 = exploring nodes, 1 = showing path
    animationSpeed = 100; // ms between steps
    
    std::cout << "BFS path found with " << fullAlgorithmPath.size() 
              << " steps, explored " << fullExploredNodes.size() << " nodes" << std::endl;
    
    // Set up initial visualization (show player at start, exit at exit)
    setupAlgorithmVisualization();
}


    
    void runDFSAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "DFS: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        isAnimating = true;
        currentAlgorithm = 2;  // DFS
        
        const auto& initialState = gameState.getInitialState();
        auto result = DungeonAlgorithms::dfsSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        fullAlgorithmPath = result.path;
        fullExploredNodes = result.exploredNodes;
        
        // Initialize animation state
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        animationSpeed = 100;
        
        std::cout << "DFS path found with " << fullAlgorithmPath.size() 
                  << " steps, explored " << fullExploredNodes.size() << " nodes" << std::endl;
        
        animationStartTime = std::chrono::steady_clock::now();
        lastAnimationTime = animationStartTime;
        
        setupAlgorithmVisualization();
    }
    
    void runDijkstraAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "UCS: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        isAnimating = true;
        currentAlgorithm = 3;  // Dijkstra/UCS
        
        const auto& initialState = gameState.getInitialState();
        auto result = DungeonAlgorithms::dijkstraSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        fullAlgorithmPath = result.path;
        fullExploredNodes = result.exploredNodes;
        
        // Initialize animation state
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        animationSpeed = 100;
        
        std::cout << "UCS path found with " << fullAlgorithmPath.size() 
                  << " steps, explored " << fullExploredNodes.size() << " nodes" << std::endl;
        
        animationStartTime = std::chrono::steady_clock::now();
        lastAnimationTime = animationStartTime;
        
        setupAlgorithmVisualization();
    }
    
    void runAStarAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "A*: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        isAnimating = true;
        currentAlgorithm = 4;  // A*
        
        const auto& initialState = gameState.getInitialState();
        auto result = DungeonAlgorithms::aStarSearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        fullAlgorithmPath = result.path;
        fullExploredNodes = result.exploredNodes;
        
        // Initialize animation state
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        animationSpeed = 100;
        
        std::cout << "A* path found with " << fullAlgorithmPath.size() 
                  << " steps, explored " << fullExploredNodes.size() << " nodes" << std::endl;
        
        animationStartTime = std::chrono::steady_clock::now();
        lastAnimationTime = animationStartTime;
        
        setupAlgorithmVisualization();
    }
    
    void runGreedyAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "Greedy: Game must be over to run algorithm" << std::endl;
            return;
        }
        
        algorithmRunning = true;
        isAnimating = true;
        currentAlgorithm = 5;  // Greedy
        
        const auto& initialState = gameState.getInitialState();
        auto result = DungeonAlgorithms::greedySearch(
            initialState.actualGrid,
            {initialState.playerStartX, initialState.playerStartY},
            {initialState.exitX, initialState.exitY}
        );
        
        fullAlgorithmPath = result.path;
        fullExploredNodes = result.exploredNodes;
        
        // Initialize animation state
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        animationSpeed = 100;
        
        std::cout << "Greedy path found with " << fullAlgorithmPath.size() 
                  << " steps, explored " << fullExploredNodes.size() << " nodes" << std::endl;
        
        animationStartTime = std::chrono::steady_clock::now();
        lastAnimationTime = animationStartTime;
        
        setupAlgorithmVisualization();
    }
    
    // Animation control methods
void startAnimation() {
    if (algorithmRunning && !isAnimating) {
        isAnimating = true;
        lastAnimationTime = std::chrono::steady_clock::now();
        std::cout << "Animation started" << std::endl;
        reDraw();
    }
}
    
    void stopAnimation() {
        isAnimating = false;
        std::cout << "Animation stopped" << std::endl;
    }
    
void pauseAnimation() {
    if (algorithmRunning && isAnimating) {
        isAnimating = false;
        std::cout << "Animation paused" << std::endl;
        reDraw();
    }
}
    
void stepAnimation() {
    if (algorithmRunning) {
        if (animationPhase == 0) {
            // Step through explored nodes
            if (currentExploredIndex < fullExploredNodes.size()) {
                currentExploredIndex++;
            } else {
                animationPhase = 1; // Move to path animation
                std::cout << "Switching to path animation" << std::endl;
            }
        } else if (animationPhase == 1) {
            // Step through path
            if (currentPathIndex < fullAlgorithmPath.size()) {
                currentPathIndex++;
            } else {
                // Animation complete
                std::cout << "Animation complete" << std::endl;
            }
        }
        reDraw();
    }
}

    
 void updateAnimation() {
    if (!isAnimating || !algorithmRunning) return;
    
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - lastAnimationTime).count();
    
    // Update animation based on elapsed time
    if (elapsed >= animationSpeed) {
        if (animationPhase == 0) {
            // Animate explored nodes
            if (currentExploredIndex < fullExploredNodes.size()) {
                currentExploredIndex++;
                lastAnimationTime = currentTime;
                reDraw();
            } else {
                animationPhase = 1; // Switch to path animation
                lastAnimationTime = currentTime;
            }
        } else if (animationPhase == 1) {
            // Animate path
            if (currentPathIndex < fullAlgorithmPath.size()) {
                currentPathIndex++;
                lastAnimationTime = currentTime;
                reDraw();
            } else {
                // Animation complete
                isAnimating = false;
                std::cout << "Animation complete" << std::endl;
            }
        }
    }
}
    
    void resetAlgorithmVisualization() {
        algorithmRunning = false;
        isAnimating = false;
        currentAlgorithm = 0;
        fullAlgorithmPath.clear();
        fullExploredNodes.clear();
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        
        // Reset to current game state
        gameState.resetVisualization();
        std::cout << "Algorithm visualization reset" << std::endl;
        reDraw();
    }
    
    // NEW: Toggle explored nodes visibility
    void toggleExploredNodes() {
        showExploredNodes = !showExploredNodes;
        reDraw();
    }
    
private:
    void setupAlgorithmVisualization() {
        // Set up the base visualization with player at start and exit at exit
        const auto& initialState = gameState.getInitialState();
        
        // Reset to initial state for algorithm visualization
        for (int i = 0; i < GameState::GRID_SIZE; i++) {
            for (int j = 0; j < GameState::GRID_SIZE; j++) {
                displayGridForAnimation[i][j] = initialState.actualGrid[i][j];
            }
        }
        
        // Make sure player is at start position for algorithm visualization
        displayGridForAnimation[initialState.playerStartX][initialState.playerStartY] = GameState::PLAYER;
        displayGridForAnimation[initialState.exitX][initialState.exitY] = GameState::EXIT;
        
        reDraw();
    }
    
    void updateVisualization() {
        const auto& initialState = gameState.getInitialState();
        
        // Copy initial state
        for (int i = 0; i < GameState::GRID_SIZE; i++) {
            for (int j = 0; j < GameState::GRID_SIZE; j++) {
                displayGridForAnimation[i][j] = initialState.actualGrid[i][j];
            }
        }
        
        // Add explored nodes
        if (showExploredNodes) {
            for (int i = 0; i < currentExploredIndex && i < fullExploredNodes.size(); i++) {
                int x = fullExploredNodes[i].first;
                int y = fullExploredNodes[i].second;
                // Don't overwrite start, exit, or obstacles
                if (!(x == initialState.playerStartX && y == initialState.playerStartY) &&
                    !(x == initialState.exitX && y == initialState.exitY)) {
                    int cellType = initialState.actualGrid[x][y];
                    if (cellType < GameState::REWARD || cellType > GameState::MINE) {
                        displayGridForAnimation[x][y] = GameState::EXPLORED_NODE;
                    }
                }
            }
        }
        
        // Add path nodes
        for (int i = 0; i < currentPathIndex && i < fullAlgorithmPath.size(); i++) {
            int x = fullAlgorithmPath[i].first;
            int y = fullAlgorithmPath[i].second;
            if (!(x == initialState.playerStartX && y == initialState.playerStartY) &&
                !(x == initialState.exitX && y == initialState.exitY)) {
                displayGridForAnimation[x][y] = GameState::PATH_VISUAL;
            }
        }
        
        // Always show player at start and exit at exit
        displayGridForAnimation[initialState.playerStartX][initialState.playerStartY] = GameState::PLAYER;
        displayGridForAnimation[initialState.exitX][initialState.exitY] = GameState::EXIT;
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
        
        // Handle ASCII keys (WASD and algorithm triggers)
        if (key.isASCII()) {
            char ch = key.getChar();
            
            // WASD movement keys
            if (ch == 'w' || ch == 'W') {
                bool moved = gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() - 1);
                if (moved) reDraw();
                return true;
            }
            if (ch == 's' || ch == 'S') {
                bool moved = gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() + 1);
                if (moved) reDraw();
                return true;
            }
            
            // Animation control keys - updated for new behavior
            if (ch == 'p' || ch == 'P') {
                if (algorithmRunning) {
                    if (isAnimating) {
                        pauseAnimation();
                    } else {
                        startAnimation();
                    }
                }
                return true;
            }
            
            if (ch == 't' || ch == 'T') {
                if (algorithmRunning && !isAnimating) {  // Only step when paused
                    stepAnimation();
                }
                return true;
            }
            
            // Algorithm triggers (only when game is over)
            if (gameState.isGameOver()) {
                switch(ch) {
                    case 'a': case 'A':
                        runAStarAlgorithm(); 
                        return true;
                    case 'd': case 'D':
                        runDFSAlgorithm(); 
                        return true;
                    case '1': case 'b': case 'B': 
                        runBFSAlgorithm(); 
                        return true;
                    case '2':
                        runDFSAlgorithm(); 
                        return true;
                    case '3': case 'j': case 'J': 
                        runDijkstraAlgorithm(); 
                        return true;
                    case '4':
                        runAStarAlgorithm(); 
                        return true;
                    case '5': case 'g': case 'G': 
                        runGreedyAlgorithm(); 
                        return true;
                }
            } else {
                // Movement keys when game is not over
                if (ch == 'a' || ch == 'A') {
                    bool moved = gameState.movePlayer(gameState.getPlayerX() - 1, gameState.getPlayerY());
                    if (moved) reDraw();
                    return true;
                }
                if (ch == 'd' || ch == 'D') {
                    bool moved = gameState.movePlayer(gameState.getPlayerX() + 1, gameState.getPlayerY());
                    if (moved) reDraw();
                    return true;
                }
            }
            
            switch(ch) {
                case 'r': case 'R': 
                    resetAlgorithmVisualization(); 
                    return true;
                case 'n': case 'N':  // New game shortcut for testing
                    resetGame();
                    return true;
                case 'e': case 'E':  // Toggle explored nodes
                    toggleExploredNodes();
                    return true;
                case ' ':  // Space bar to toggle play/pause
                    if (algorithmRunning) {
                        if (isAnimating) {
                            pauseAnimation();
                        } else {
                            startAnimation();
                        }
                    }
                    return true;
            }
        }
        
        return gui::Canvas::onKeyPressed(key);
    }
    
void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override {
    gui::Point clickPos = inputDevice.getModelPoint();
    
    // Check if clicked on dropdown
    if (dropdownRect.contains(clickPos)) {
        dropdownExpanded = !dropdownExpanded;
        reDraw();
        return;
    }
    
    // Check if clicked on dropdown menu items
    if (dropdownExpanded) {
        for (int i = 0; i < 5; i++) {
            if (dropdownItemRects[i].contains(clickPos)) {
                currentAlgorithm = i + 1;
                dropdownExpanded = false;
                
                // Run the selected algorithm if game is over
                if (gameState.isGameOver()) {
                    switch (currentAlgorithm) {
                        case 1: runBFSAlgorithm(); break;
                        case 2: runDFSAlgorithm(); break;
                        case 3: runDijkstraAlgorithm(); break;
                        case 4: runAStarAlgorithm(); break;
                        case 5: runGreedyAlgorithm(); break;
                    }
                }
                
                reDraw();
                return;
            }
        }
        
        // Close dropdown if clicked elsewhere
        dropdownExpanded = false;
        reDraw();
        return;
    }
    
    // Check if clicked on "Start Animation" button
    if (startButtonRect.contains(clickPos) && algorithmRunning && !isAnimating) {
        startAnimation();  // Start animation from beginning
        return;
    }
    
    // Check if clicked on "Pause Animation" button
    if (pauseButtonRect.contains(clickPos) && algorithmRunning && isAnimating) {
        pauseAnimation();  // Stop animation where it is
        return;
    }
    
    // Check if clicked on "Step" button
    if (stepButtonRect.contains(clickPos) && algorithmRunning) {
        stepAnimation();  // Step forward one frame
        return;
    }
    
    // Check if clicked on "Generate New Game" button
    if (generateNewGameRect.contains(clickPos)) {
        resetGame();
        return;
    }
    
    // Check if clicked on "Reset" button
    if (resetButtonRect.contains(clickPos) && algorithmRunning) {
        resetAlgorithmVisualization();
        return;
    }
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
        isAnimating = false;
        currentAlgorithm = 0;
        fullAlgorithmPath.clear();
        fullExploredNodes.clear();
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        dropdownExpanded = false;
        showExploredNodes = true;
        std::cout << "New game started" << std::endl;
        reDraw();
    }
    
    // Handle game events (like mine hits) - KEEPING ORIGINAL VERSION WITH QuestionsPopUp
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
        
        gui::CoordType zoneGap = newSize.width * 0.015;
        leftZoneLeft = newSize.width * 0.03;
        leftZoneTop = (newSize.height - leftZoneHeight) / 2;
        
        rightZoneLeft = leftZoneLeft + leftZoneWidth + zoneGap;
        rightZoneWidth = newSize.width - rightZoneLeft - (newSize.width * 0.03);
        rightZoneHeight = newSize.height * 0.9;
        rightZoneTop = (newSize.height - rightZoneHeight) / 2;
        
        this->reDraw();
    }
    
    void onDraw(const gui::Rect& rect) override {
        // Update animation before drawing
        if (algorithmRunning && isAnimating) {
            updateAnimation();
        }
        
        // Update visualization if algorithm is running
        if (algorithmRunning) {
            updateVisualization();
        }
        
        // Draw background
        gui::Shape bg;
        bg.createRect(rect);
        bg.drawFill(td::ColorID::Moss);
        
        // Draw left zone (game grid)
        drawGameGrid();
        
        // Draw right zone (control panel)
        drawControlPanel();
    }
    
private:
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
            }
            catch (...) {
                backgroundLoaded = false;
                gui::Shape gridBg;
                gridBg.createRect(gui::Rect(gridStartX, gridStartY,
                    gridStartX + gridAreaSize,
                    gridStartY + gridAreaSize));
                gridBg.drawFill(td::ColorID::DarkGray);
            }
        }
        else {
            gui::Shape gridBg;
            gridBg.createRect(gui::Rect(gridStartX, gridStartY,
                gridStartX + gridAreaSize,
                gridStartY + gridAreaSize));
            gridBg.drawFill(td::ColorID::DarkGray);
        }
        
        // Draw grid lines
        for (int j = 0; j <= GRID_SIZE; j++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = { gridStartX, gridStartY + j * cellSize };
            points[1] = { gridStartX + gridAreaSize, gridStartY + j * cellSize };
            line.createLines(points, 2, 2);
            line.drawWire(td::ColorID::Gray);
        }
        
        for (int i = 0; i <= GRID_SIZE; i++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = { gridStartX + i * cellSize, gridStartY };
            points[1] = { gridStartX + i * cellSize, gridStartY + gridAreaSize };
            line.createLines(points, 2, 2);
            line.drawWire(td::ColorID::Gray);
        }
        
        // Draw cells - use algorithm visualization if running, otherwise use game state
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                int cellType;
                if (algorithmRunning) {
                    cellType = displayGridForAnimation[i][j];
                } else {
                    cellType = gameState.getDisplayCell(i, j);
                }
                
                if (cellType != GameState::EMPTY) {
                    drawCellContent(gridStartX + i * cellSize,
                        gridStartY + j * cellSize,
                        cellSize, cellType);
                }
            }
        }
        
        // Draw animation indicator if animating
        if (isAnimating) {
            drawAnimationIndicator(gridStartX, gridStartY, gridAreaSize);
        }
    }
    
    void drawAnimationIndicator(gui::CoordType x, gui::CoordType y, gui::CoordType size) {
        // Draw a pulsing border around the grid to indicate animation is running
        static float pulse = 0.0f;
        pulse += 0.1f;
        if (pulse > 2 * 3.14159f) pulse = 0.0f;
        
        gui::Shape border;
        border.createRect(gui::Rect(x, y, x + size, y + size));
        border.drawWire(td::ColorID::Yellow, 3.0f);
        
        // Draw animation status text
        std::string status;
        if (animationPhase == 0) {
            status = "Exploring: " + std::to_string(currentExploredIndex) + "/" + std::to_string(fullExploredNodes.size());
        } else {
            status = "Path: " + std::to_string(currentPathIndex) + "/" + std::to_string(fullAlgorithmPath.size());
        }
        
        gui::DrawableString::draw(status.c_str(), status.length(),
            gui::Rect(x, y - 25, x + size, y),
            gui::Font::ID::SystemSmaller,
            td::ColorID::Yellow,
            td::TextAlignment::Center,
            td::VAlignment::Bottom);
    }
    
    void drawCellContent(gui::CoordType x, gui::CoordType y,
                        gui::CoordType size, int cellType) {
        gui::CoordType margin = size * 0.1;
        gui::Rect cellRect(x + margin, y + margin,
                          x + size - margin, y + size - margin);
        
        // Handle explored nodes
        if (cellType == GameState::EXPLORED_NODE && showExploredNodes) {
            gui::Shape exploredShape;
            exploredShape.createRect(cellRect);
            exploredShape.drawFill(td::ColorID::LightBlue);
            
            // Draw a smaller inner shape for better visibility
            gui::Shape innerShape;
            gui::Rect innerRect(x + margin + 2, y + margin + 2,
                               x + size - margin - 2, y + size - margin - 2);
            innerShape.createRect(innerRect);
            innerShape.drawFill(td::ColorID::SkyBlue);
            return;
        }
        
        // Handle algorithm path
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
    
    void drawControlPanel() {
        gui::CoordType panelX = rightZoneLeft;
        gui::CoordType panelY = rightZoneTop;
        gui::CoordType panelWidth = rightZoneWidth;
        gui::CoordType currentY = panelY + 15;
        
        // Title
        drawSectionTitle("Select Algorithm:", panelX, currentY, panelWidth);
        currentY += 45;
        
        // Save dropdown position but DON'T draw it yet
        gui::CoordType dropdownY = currentY;
        currentY += 70;
        
        // Statistics panel
        drawStatisticsPanel(panelX, currentY, panelWidth);
        currentY += 165;
        
        // Control buttons - UPDATED with animation controls
        drawControlButtons(panelX, currentY, panelWidth);
        currentY += 190;
        
        // Algorithm comparison table
        drawComparisonTable(panelX, currentY, panelWidth);
        
        // Draw dropdown LAST so it appears on top of everything
        drawAlgorithmDropdown(panelX, dropdownY, panelWidth);
        
        // Draw dropdown menu AFTER dropdown if expanded
        if (dropdownExpanded) {
            drawDropdownMenu(panelX, dropdownY + 53, panelWidth);
        }
    }
    
    void drawSectionTitle(const char* title, gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::DrawableString::draw(title, strlen(title),
            gui::Rect(x, y, x + width, y + 30),
            gui::Font::ID::SystemNormal,
            td::ColorID::White,
            td::TextAlignment::Left,
            td::VAlignment::Center);
    }
    
    void drawAlgorithmDropdown(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        // Store dropdown rectangle for click detection
        dropdownRect = gui::Rect(x, y, x + width, y + 50);
        
        // Dropdown background
        gui::Shape selectorBg;
        selectorBg.createRoundedRect(dropdownRect, 6);
        selectorBg.drawFill(td::ColorID::Moss);
        
        // Outline
        gui::Shape selectorBorder;
        selectorBorder.createRoundedRect(dropdownRect, 6);
        selectorBorder.drawWire(td::ColorID::LightGreen, 2);
        
        std::string algoName;
        switch (currentAlgorithm) {
            case 0: algoName = "Select Algorithm..."; break;
            case 1: algoName = "Breadth-First Search (BFS)"; break;
            case 2: algoName = "Depth-First Search (DFS)"; break;
            case 3: algoName = "Uniform Cost Search (UCS)"; break;
            case 4: algoName = "A* Search"; break;
            case 5: algoName = "Greedy Best-First Search"; break;
        }
        
        gui::DrawableString::draw(algoName.c_str(), algoName.length(),
            gui::Rect(x + 15, y, x + width - 40, y + 50),
            gui::Font::ID::SystemNormal,
            td::ColorID::White,
            td::TextAlignment::Left,
            td::VAlignment::Center);
        
        // Dropdown arrow
        const char* arrow = dropdownExpanded ? "^" : "v";
        gui::DrawableString::draw(arrow, strlen(arrow),
            gui::Rect(x + width - 35, y, x + width - 10, y + 50),
            gui::Font::ID::SystemBold,
            td::ColorID::White,
            td::TextAlignment::Center,
            td::VAlignment::Center);
    }
    
    void drawDropdownMenu(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        const char* options[] = {
            "Breadth-First Search (BFS)",
            "Depth-First Search (DFS)",
            "Uniform Cost Search (UCS)",
            "A* Search",
            "Greedy Best-First Search"
        };
        
        gui::CoordType itemHeight = 45;
        gui::CoordType menuHeight = 5 * itemHeight;
        
        // Menu background
        gui::Shape menuBg;
        gui::Rect menuRect(x, y, x + width, y + menuHeight);
        menuBg.createRoundedRect(menuRect, 6);
        menuBg.drawFill(td::ColorID::Moss);
        
        // Border
        gui::Shape menuBorder;
        menuBorder.createRoundedRect(menuRect, 6);
        menuBorder.drawWire(td::ColorID::LightGreen, 2);
        
        // Draw each option and store rectangles
        for (int i = 0; i < 5; i++) {
            gui::CoordType itemY = y + i * itemHeight;
            dropdownItemRects[i] = gui::Rect(x, itemY, x + width, itemY + itemHeight);
            
            // Highlight if selected
            if (i + 1 == currentAlgorithm) {
                gui::Shape highlight;
                highlight.createRect(gui::Rect(x + 3, itemY + 2, x + width - 3, itemY + itemHeight - 2));
                highlight.drawFill(td::ColorID::DarkRed);
            }
            
            gui::DrawableString::draw(options[i], strlen(options[i]),
                gui::Rect(x + 15, itemY, x + width - 15, itemY + itemHeight),
                gui::Font::ID::SystemNormal,
                td::ColorID::White,
                td::TextAlignment::Left,
                td::VAlignment::Center);
        }
    }
    
    void drawStatisticsPanel(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        // Stats container
        gui::Shape statsBg;
        gui::Rect statsRect(x, y, x + width, y + 150);
        statsBg.createRoundedRect(statsRect, 6);
        statsBg.drawFill(td::ColorID::Moss);
        
        // Border
        gui::Shape statsBorder;
        statsBorder.createRoundedRect(statsRect, 6);
        statsBorder.drawWire(td::ColorID::LightGreen, 2);
        
        gui::CoordType currentY = y + 20;
        
        // Gold and Status row
        std::string status;
        if (isAnimating) {
            status = "Animating";
        } else if (algorithmRunning) {
            status = "Paused";
        } else {
            status = gameState.isGameOver() ? 
                (gameState.isGameWon() ? "Reached the Exit!" : "Game Over") : "Playing";
        }
        
        drawStatRow("Current Gold", std::to_string(gameState.getGold()),
            "Status", status,
            x + 20, currentY, width - 40);
        currentY += 65;
        
        // Path Progress and Nodes Explored row
        std::string pathProgress = algorithmRunning ? 
            std::to_string(currentPathIndex) + "/" + std::to_string(fullAlgorithmPath.size()) :
            "0";
        
        std::string exploredProgress = algorithmRunning ? 
            std::to_string(currentExploredIndex) + "/" + std::to_string(fullExploredNodes.size()) :
            "0";
        
        drawStatRow("Path Progress", pathProgress,
            "Explored Nodes", exploredProgress,
            x + 20, currentY, width - 40);
    }
    
    void drawStatRow(const char* label1, const std::string& value1,
                    const char* label2, const std::string& value2,
                    gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::CoordType halfWidth = width / 2;
        
        // Left stat
        gui::DrawableString::draw(label1, strlen(label1),
            gui::Rect(x, y, x + halfWidth - 15, y + 22),
            gui::Font::ID::SystemNormal,
            td::ColorID::LightGray,
            td::TextAlignment::Left,
            td::VAlignment::Center);
        
        gui::DrawableString::draw(value1.c_str(), value1.length(),
            gui::Rect(x, y + 25, x + halfWidth - 15, y + 50),
            gui::Font::ID::SystemBold,
            td::ColorID::Yellow,
            td::TextAlignment::Left,
            td::VAlignment::Center);
        
        // Right stat
        gui::DrawableString::draw(label2, strlen(label2),
            gui::Rect(x + halfWidth + 15, y, x + width, y + 22),
            gui::Font::ID::SystemNormal,
            td::ColorID::LightGray,
            td::TextAlignment::Right,
            td::VAlignment::Center);
        
        gui::DrawableString::draw(value2.c_str(), value2.length(),
            gui::Rect(x + halfWidth + 15, y + 25, x + width, y + 50),
            gui::Font::ID::SystemBold,
            td::ColorID::LightGreen,
            td::TextAlignment::Right,
            td::VAlignment::Center);
    }
    
    void drawControlButtons(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::CoordType buttonHeight = 45;
        gui::CoordType buttonSpacing = 12;
        gui::CoordType currentY = y;
        
        // Row 1: START and PAUSE
        startButtonRect = gui::Rect(x, currentY, x + width / 2 - 6, currentY + buttonHeight);
        drawRoundedButton("START", x, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, algorithmRunning && !isAnimating, startButtonRect);
        
        pauseButtonRect = gui::Rect(x + width / 2 + 6, currentY, x + width, currentY + buttonHeight);
        drawRoundedButton("PAUSE", x + width / 2 + 6, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, algorithmRunning && isAnimating, pauseButtonRect);
        currentY += buttonHeight + buttonSpacing;
        
        // Row 2: STEP and RESET
        stepButtonRect = gui::Rect(x, currentY, x + width / 2 - 6, currentY + buttonHeight);
        drawRoundedButton("STEP", x, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, algorithmRunning && !isAnimating, stepButtonRect);
        
        resetButtonRect = gui::Rect(x + width / 2 + 6, currentY, x + width, currentY + buttonHeight);
        drawRoundedButton("RESET", x + width / 2 + 6, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, algorithmRunning, resetButtonRect);
        currentY += buttonHeight + buttonSpacing;
        
        // Row 3: GENERATE NEW GAME
        generateNewGameRect = gui::Rect(x, currentY, x + width, currentY + buttonHeight);
        drawRoundedButton("GENERATE NEW DUNGEON", x, currentY, width, buttonHeight,
            td::ColorID::Copper, true, generateNewGameRect);
        currentY += buttonHeight + buttonSpacing;
        
    }
    
    void drawRoundedButton(const char* label, gui::CoordType x, gui::CoordType y,
                          gui::CoordType width, gui::CoordType height,
                          td::ColorID color, bool enabled, const gui::Rect& clickRect) {
        gui::Shape buttonBg;
        gui::Rect buttonRect(x, y, x + width, y + height);
        buttonBg.createRoundedRect(buttonRect, 6);
        buttonBg.drawFill(enabled ? color : td::ColorID::DimGray);
        
        // Border
        gui::Shape buttonBorder;
        buttonBorder.createRoundedRect(buttonRect, 6);
        buttonBorder.drawWire(enabled ? td::ColorID::Gray : td::ColorID::DarkGray, 1);
        
        // Text
        gui::DrawableString::draw(label, strlen(label),
            gui::Rect(x, y, x + width, y + height),
            gui::Font::ID::SystemNormal,
            td::ColorID::White,
            td::TextAlignment::Center,
            td::VAlignment::Center);
    }
    
    void drawComparisonTable(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        // Title
        gui::DrawableString::draw("Algorithm Comparison", 21,
            gui::Rect(x, y, x + width, y + 30),
            gui::Font::ID::SystemNormal,
            td::ColorID::White,
            td::TextAlignment::Left,
            td::VAlignment::Center);
        
        y += 35;
        
        // Table background
        gui::Shape tableBg;
        gui::Rect tableRect(x, y, x + width, y + 145);
        tableBg.createRoundedRect(tableRect, 6);
        tableBg.drawFill(td::ColorID::Moss);
        
        // Border
        gui::Shape tableBorder;
        tableBorder.createRoundedRect(tableRect, 6);
        tableBorder.drawWire(td::ColorID::LightGreen, 2);
        
        // Header row
        gui::CoordType headerY = y + 15;
        drawTableHeader(x + 15, headerY, width - 30);
        
        // Animation speed control message
        const char* speedMsg = "Animation Speed: Medium (100ms/step)";
        gui::DrawableString::draw(speedMsg, strlen(speedMsg),
            gui::Rect(x + 20, headerY + 40, x + width - 20, headerY + 95),
            gui::Font::ID::SystemSmaller,
            td::ColorID::LightGray,
            td::TextAlignment::Center,
            td::VAlignment::Center);
        
        // Controls hint
        const char* controls = "Press SPACE to play/pause, T to step (when paused)";
        gui::DrawableString::draw(controls, strlen(controls),
            gui::Rect(x + 20, headerY + 100, x + width - 20, headerY + 130),
            gui::Font::ID::SystemSmaller,
            td::ColorID::LightGray,
            td::TextAlignment::Center,
            td::VAlignment::Center);
    }
    
    void drawTableHeader(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::CoordType col1 = width * 0.35;
        gui::CoordType col2 = width * 0.20;
        gui::CoordType col3 = width * 0.20;
        gui::CoordType col4 = width * 0.25;
        
        const char* headers[] = { "Algorithm", "Time", "Gold", "Efficiency" };
        gui::CoordType positions[] = { x, x + col1, x + col1 + col2, x + col1 + col2 + col3 };
        
        for (int i = 0; i < 4; i++) {
            gui::CoordType w = (i == 0) ? col1 : (i == 1) ? col2 : (i == 2) ? col3 : col4;
            gui::DrawableString::draw(headers[i], strlen(headers[i]),
                gui::Rect(positions[i], y, positions[i] + w, y + 22),
                gui::Font::ID::SystemSmaller,
                td::ColorID::White,
                td::TextAlignment::Left,
                td::VAlignment::Center);
        }
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
    std::vector<std::pair<int, int>> fullAlgorithmPath;
    std::vector<std::pair<int, int>> fullExploredNodes;
    int currentAlgorithm = 0;  // 0=none, 1=BFS, 2=DFS, 3=Dijkstra, 4=A*, 5=Greedy
    
    // Animation state
    bool isAnimating = false;
    int currentExploredIndex = 0;
    int currentPathIndex = 0;
    int animationPhase = 0;  // 0 = exploring nodes, 1 = showing path
    int animationSpeed = 50; // ms between steps
    std::chrono::steady_clock::time_point animationStartTime;
    std::chrono::steady_clock::time_point lastAnimationTime;
    
    // Toggle for explored nodes visibility
    bool showExploredNodes = true;
    bool dropdownExpanded = false;
    
    // Separate display grid for algorithm visualization
    int displayGridForAnimation[GameState::GRID_SIZE][GameState::GRID_SIZE];
    
    // Button rectangles for click detection
    gui::Rect dropdownRect;
    gui::Rect dropdownItemRects[5];
    gui::Rect generateNewGameRect;
    gui::Rect resetButtonRect;
    gui::Rect startButtonRect;
    gui::Rect pauseButtonRect;
    gui::Rect stepButtonRect;
};
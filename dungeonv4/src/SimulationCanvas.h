// SimulationCanvas.h
#pragma once
#include <vector>   
#include <utility>     
#include <cstring>  
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Image.h>
#include <gui/Button.h>
#include <gui/ComboBox.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include "Algorithms.h"
#include "GameState.h"

class SimulationCanvas : public gui::Canvas {
public:
    SimulationCanvas()
        : gui::Canvas({ gui::InputDevice::Event::Keyboard, gui::InputDevice::Event::PrimaryClicks })
        , rng(std::random_device{}())
        , gameState(rng)
        , imgPlayer(":player")
        , imgReward(":reward")
        , imgBandit(":bandit")
        , imgMine(":mine")
        , imgExit(":exit")
        , imgBackground(":background")
        , imgPath(":path") {
        enableResizeEvent(true);
        std::cout << "SimulationCanvas initialized successfully" << std::endl;
    }

    ~SimulationCanvas() = default;

    // Game state queries
    bool isGameOver() const { return gameState.isGameOver(); }
    bool isGameWon() const { return gameState.isGameWon(); }
    int getGold() const { return gameState.getGold(); }
    std::pair<int, int> getPlayerPosition() const {
        return { gameState.getPlayerX(), gameState.getPlayerY() };
    }

    // Algorithm methods
    void runBFSAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "BFS: Game must be over to run algorithm" << std::endl;
            return;
        }

        algorithmRunning = true;
        currentAlgorithm = 1;

        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::bfsSearch(
            initialState.actualGrid,
            { initialState.playerStartX, initialState.playerStartY },
            { initialState.exitX, initialState.exitY }
        );

        pathLength = algorithmPath.size();
        nodesExplored = calculateNodesExplored(1);

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
        currentAlgorithm = 2;

        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::dfsSearch(
            initialState.actualGrid,
            { initialState.playerStartX, initialState.playerStartY },
            { initialState.exitX, initialState.exitY }
        );

        pathLength = algorithmPath.size();
        nodesExplored = calculateNodesExplored(2);

        std::cout << "DFS path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }

    void runDijkstraAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "UCS: Game must be over to run algorithm" << std::endl;
            return;
        }

        algorithmRunning = true;
        currentAlgorithm = 3;

        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::dijkstraSearch(
            initialState.actualGrid,
            { initialState.playerStartX, initialState.playerStartY },
            { initialState.exitX, initialState.exitY }
        );

        pathLength = algorithmPath.size();
        nodesExplored = calculateNodesExplored(3);

        std::cout << "UCS path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }

    void runAStarAlgorithm() {
        if (!gameState.isGameOver()) {
            std::cout << "A*: Game must be over to run algorithm" << std::endl;
            return;
        }

        algorithmRunning = true;
        currentAlgorithm = 4;

        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::aStarSearch(
            initialState.actualGrid,
            { initialState.playerStartX, initialState.playerStartY },
            { initialState.exitX, initialState.exitY }
        );

        pathLength = algorithmPath.size();
        nodesExplored = calculateNodesExplored(4);

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
        currentAlgorithm = 5;

        const auto& initialState = gameState.getInitialState();
        algorithmPath = DungeonAlgorithms::greedySearch(
            initialState.actualGrid,
            { initialState.playerStartX, initialState.playerStartY },
            { initialState.exitX, initialState.exitY }
        );

        pathLength = algorithmPath.size();
        nodesExplored = calculateNodesExplored(5);

        std::cout << "Greedy path found with " << algorithmPath.size() << " steps" << std::endl;
        gameState.visualizePath(algorithmPath);
        reDraw();
    }

    void resetAlgorithmVisualization() {
        algorithmRunning = false;
        currentAlgorithm = 0;
        algorithmPath.clear();
        pathLength = 0;
        nodesExplored = 0;
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

            // Algorithm triggers (only when game is over)
            switch (ch) {
            case 'a': case 'A':
                if (!gameState.isGameOver()) {
                    // Movement left
                    bool moved = gameState.movePlayer(gameState.getPlayerX() - 1, gameState.getPlayerY());
                    if (moved) reDraw();
                }
                else {
                    // A* algorithm
                    runAStarAlgorithm();
                }
                return true;
            case 'd': case 'D':
                if (!gameState.isGameOver()) {
                    // Movement right
                    bool moved = gameState.movePlayer(gameState.getPlayerX() + 1, gameState.getPlayerY());
                    if (moved) reDraw();
                }
                else {
                    // DFS algorithm
                    runDFSAlgorithm();
                }
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
            case 'r': case 'R':
                resetAlgorithmVisualization();
                return true;
            case 'n': case 'N':
                resetGame();
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
        rng = std::mt19937(std::random_device{}());
        gameState = GameState(rng);
        algorithmRunning = false;
        currentAlgorithm = 0;
        algorithmPath.clear();
        pathLength = 0;
        nodesExplored = 0;
        dropdownExpanded = false;
        std::cout << "New game started" << std::endl;
        reDraw();
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
    // Helper function to calculate nodes explored (estimation)
    int calculateNodesExplored(int algorithmType) {
        // This is a simplified estimation
        switch (algorithmType) {
        case 1: return pathLength * 2;      // BFS explores more
        case 2: return pathLength * 3;      // DFS can explore many dead ends
        case 3: return pathLength * 2;      // Dijkstra similar to BFS
        case 4: return pathLength;          // A* is efficient
        case 5: return pathLength;          // Greedy is fast
        default: return 0;
        }
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

        if (cellType == GameState::PATH_VISUAL) {
            gui::Shape pathShape;
            pathShape.createRect(cellRect);
            pathShape.drawFill(td::ColorID::Purple);
            try {
                imgPath.draw(cellRect);
            }
            catch (...) {}
            return;
        }

        if (imagesLoaded) {
            try {
                switch (cellType) {
                case GameState::PLAYER: imgPlayer.draw(cellRect); break;
                case GameState::REWARD: imgReward.draw(cellRect); break;
                case GameState::BANDIT: imgBandit.draw(cellRect); break;
                case GameState::MINE: imgMine.draw(cellRect); break;
                case GameState::EXIT: imgExit.draw(cellRect); break;
                }
                return;
            }
            catch (...) {
                imagesLoaded = false;
            }
        }

        // Fallback to colored squares
        gui::Shape cellShape;
        cellShape.createRect(cellRect);

        switch (cellType) {
        case GameState::PLAYER: cellShape.drawFill(td::ColorID::Green); break;
        case GameState::REWARD: cellShape.drawFill(td::ColorID::Yellow); break;
        case GameState::BANDIT: cellShape.drawFill(td::ColorID::Blue); break;
        case GameState::MINE: cellShape.drawFill(td::ColorID::Red); break;
        case GameState::EXIT: cellShape.drawFill(td::ColorID::White); break;
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

        // Control buttons
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

        // Dropdown background - gray-green like grid
        gui::Shape selectorBg;
        selectorBg.createRoundedRect(dropdownRect, 6);
        selectorBg.drawFill(td::ColorID::Moss);

        // Red outline
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

        // Menu background - gray-green
        gui::Shape menuBg;
        gui::Rect menuRect(x, y, x + width, y + menuHeight);
        menuBg.createRoundedRect(menuRect, 6);
        menuBg.drawFill(td::ColorID::Moss);

        // Red border
        gui::Shape menuBorder;
        menuBorder.createRoundedRect(menuRect, 6);
        menuBorder.drawWire(td::ColorID::LightGreen, 2);

        // Draw each option and store rectangles
        for (int i = 0; i < 5; i++) {
            gui::CoordType itemY = y + i * itemHeight;
            dropdownItemRects[i] = gui::Rect(x, itemY, x + width, itemY + itemHeight);

            // Highlight if selected - with red
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
        // Stats container - gray-green
        gui::Shape statsBg;
        gui::Rect statsRect(x, y, x + width, y + 150);
        statsBg.createRoundedRect(statsRect, 6);
        statsBg.drawFill(td::ColorID::Moss);

        // Red border
        gui::Shape statsBorder;
        statsBorder.createRoundedRect(statsRect, 6);
        statsBorder.drawWire(td::ColorID::LightGreen, 2);

        gui::CoordType currentY = y + 20;

        // Gold and Status row
        drawStatRow("Current Gold", std::to_string(gameState.getGold()),
            "Status", gameState.isGameOver() ? (gameState.isGameWon() ? "Reached the Exit!" : "Game Over") : "Playing",
            x + 20, currentY, width - 40);
        currentY += 65;

        // Path Length and Nodes Explored row
        drawStatRow("Path Length", std::to_string(pathLength),
            "Nodes Explored", std::to_string(nodesExplored),
            x + 20, currentY, width - 40);
    }

    void drawStatRow(const char* label1, const std::string& value1,
        const char* label2, const std::string& value2,
        gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::CoordType halfWidth = width / 2;

        // Left stat - more spacing
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

        // Right stat - more spacing
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

        // Row 1: START and PAUSE - gray-green
        drawRoundedButton("START", x, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, false, gui::Rect());
        drawRoundedButton("PAUSE", x + width / 2 + 6, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, false, gui::Rect());
        currentY += buttonHeight + buttonSpacing;

        // Row 2: STEP and RESET - gray-green
        drawRoundedButton("STEP", x, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, false, gui::Rect());

        resetButtonRect = gui::Rect(x + width / 2 + 6, currentY,
            x + width, currentY + buttonHeight);
        drawRoundedButton("RESET", x + width / 2 + 6, currentY, width / 2 - 6, buttonHeight,
            td::ColorID::Moss, algorithmRunning, resetButtonRect);
        currentY += buttonHeight + buttonSpacing;

        // Row 3: GENERATE NEW GAME - Dark Red
        generateNewGameRect = gui::Rect(x, currentY, x + width, currentY + buttonHeight);
        drawRoundedButton("GENERATE NEW DUNGEON", x, currentY, width, buttonHeight,
            td::ColorID::Copper,true, generateNewGameRect);
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

        // Always white text
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

        // Table background - gray-green
        gui::Shape tableBg;
        gui::Rect tableRect(x, y, x + width, y + 145);
        tableBg.createRoundedRect(tableRect, 6);
        tableBg.drawFill(td::ColorID::Moss);

        // Red border
        gui::Shape tableBorder;
        tableBorder.createRoundedRect(tableRect, 6);
        tableBorder.drawWire(td::ColorID::LightGreen, 2);

        // Header row
        gui::CoordType headerY = y + 15;
        drawTableHeader(x + 15, headerY, width - 30);

        // Message
        const char* msg = "Run algorithms to see metrics";
        gui::DrawableString::draw(msg, strlen(msg),
            gui::Rect(x + 20, headerY + 40, x + width - 20, headerY + 95),
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
    std::mt19937 rng;
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
    int currentAlgorithm = 0;
    int pathLength = 0;
    int nodesExplored = 0;
    bool dropdownExpanded = false;

    // Button rectangles for click detection
    gui::Rect dropdownRect;
    gui::Rect dropdownItemRects[5];
    gui::Rect generateNewGameRect;
    gui::Rect resetButtonRect;
};
#pragma once

#include <vector>
#include <utility>
#include <cstring>
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Image.h>
#include <gui/Alert.h>
#include <gui/Sound.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include <chrono>
#include "Algorithms.h"
#include "GameState.h"
#include "QuestionsPopUp.h"

class SimulationCanvas : public gui::Canvas {
private:
    enum class AlgorithmType { None, BFS, DFS, DIJKSTRA, AStar, Greedy, MDP };
    std::mt19937 rng;
    GameState gameState;

    gui::CoordType leftZoneLeft = 0, leftZoneTop = 0, leftZoneWidth = 0;
    gui::CoordType rightZoneLeft = 0, rightZoneTop = 0, rightZoneWidth = 0;

    gui::Image imgPlayer, imgReward, imgBandit, imgMine, imgExit, imgBackground;
    gui::Sound sndReward, sndMine, sndBandit, sndExit, sndNoExit, sndSoundtrack;

    bool imagesLoaded = true, backgroundLoaded = true, soundtrackPlaying = false;

    bool algorithmRunning = false;
    int  currentAlgorithm = 0;
    long long algorithmExecTimeUs = 0;
    std::vector<std::pair<int, int>> fullAlgorithmPath;
    std::vector<std::pair<int, int>> fullExploredNodes;

    bool isAnimating = false;
    int  animationPhase = 0, currentExploredIndex = 0, currentPathIndex = 0;
    int  animationSpeed = 100;
    const int MAX_SPEED = 500;
    std::chrono::steady_clock::time_point lastAnimationTime;

    bool showExploredNodes = true;
    bool dropdownExpanded = false, speedControlExpanded = false;

    int displayGrid[GameState::GRID_SIZE][GameState::GRID_SIZE];

    gui::Rect dropdownRect, dropdownItemRects[6];
    gui::Rect speedButtonRect, speedSliderRect;
    gui::Rect startButtonRect, pauseButtonRect, stepButtonRect, resetButtonRect, generateNewGameRect;

    struct PendingMineEvent {
        bool pending = false;
        int  value = 0;
    };
    PendingMineEvent pendingMine;

    struct PendingMineResult {
        bool pending = false;
        bool wasCorrect = false;
        int  value = 0;
    };
    PendingMineResult pendingMineResult;

    static const char* algorithmName(AlgorithmType type) {
        if (type == AlgorithmType::BFS)    return "BFS";
        if (type == AlgorithmType::DFS)    return "DFS";
        if (type == AlgorithmType::DIJKSTRA)    return "DIJSKTRA";
        if (type == AlgorithmType::AStar)  return "A*";
        if (type == AlgorithmType::Greedy) return "Greedy";
        if (type == AlgorithmType::MDP)    return "MDP";
        return "";
    }

    void setupAlgorithmVisualization() {
        const auto& s = gameState.getInitialState();
        for (int i = 0; i < GameState::GRID_SIZE; i++)
            for (int j = 0; j < GameState::GRID_SIZE; j++)
                displayGrid[i][j] = s.actualGrid[i][j];
        displayGrid[s.playerStartX][s.playerStartY] = GameState::PLAYER;
        displayGrid[s.exitX][s.exitY] = GameState::EXIT;
        reDraw();
    }

    void updateVisualization() {
        const auto& s = gameState.getInitialState();
        for (int i = 0; i < GameState::GRID_SIZE; i++)
            for (int j = 0; j < GameState::GRID_SIZE; j++)
                displayGrid[i][j] = s.actualGrid[i][j];

        if (showExploredNodes) {
            for (int i = 0; i < currentExploredIndex && i < (int)fullExploredNodes.size(); i++) {
                int x = fullExploredNodes[i].first;
                int y = fullExploredNodes[i].second;
                if (x == s.playerStartX && y == s.playerStartY) continue;
                if (x == s.exitX && y == s.exitY) continue;
                int cell = s.actualGrid[x][y];
                if (cell < GameState::REWARD || cell > GameState::MINE)
                    displayGrid[x][y] = GameState::EXPLORED_NODE;
            }
        }

        for (int i = 0; i < currentPathIndex && i < (int)fullAlgorithmPath.size(); i++) {
            int x = fullAlgorithmPath[i].first;
            int y = fullAlgorithmPath[i].second;
            if (x == s.playerStartX && y == s.playerStartY) continue;
            if (x == s.exitX && y == s.exitY) continue;
            displayGrid[x][y] = GameState::PATH_VISUAL;
        }

        displayGrid[s.playerStartX][s.playerStartY] = GameState::PLAYER;
        displayGrid[s.exitX][s.exitY] = GameState::EXIT;
    }

    void resetGame() {
        if (!gameState.isGameOver()) {
            showAlert("Game In Progress", "You must finish the current game first!");
            return;
        }

        if (gameState.hasEverReachedExit() && !gameState.hasMetRewardRequirement()) {
            td::String message;
            message.format("You only had %d gold (need 20). Try again!", gameState.getGold());
            showAlert("Insufficient Gold!", message);
            gameState.resetPlayerPosition();
            reDraw();
            return;
        }

        rng = std::mt19937(std::random_device{}());
        gameState = GameState(rng);
        gameState.setGameEventCallback([this](const std::string& event, int value) {
            handleGameEvent(event, value);
            });

        algorithmRunning = false;
        isAnimating = false;
        currentAlgorithm = 0;
        algorithmExecTimeUs = 0;
        fullAlgorithmPath.clear();
        fullExploredNodes.clear();
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        dropdownExpanded = false;
        showExploredNodes = true;
        soundtrackPlaying = false;
        pendingMine.pending = false;
        pendingMine.value = 0;
        pendingMineResult.pending = false;

        reDraw();
    }

    void handleGameEvent(const std::string& event, int value) {
        if (event == "mine") {
            sndMine.play();
            pendingMine.pending = true;
            pendingMine.value = value;
            reDraw();
        }
        else if (event == "reward") {
            sndReward.play();
            td::String message;
            message.format("You found %d gold!\nTotal gold: %d", value, gameState.getGold());
            gui::Alert::show("Reward Found!", message);
        }
        else if (event == "bandit") {
            sndBandit.play();
            td::String message;
            message.format("A bandit stole half your gold!\nRemaining gold: %d", gameState.getGold());
            gui::Alert::show("Bandit Attack!", message);
        }
        else if (event == "exit") {
            sndExit.play();
            td::String message;
            message.format("You escaped the dungeon!\nFinal gold: %d", gameState.getGold());
            gui::Alert::show("You Win!", message);
        }
        else if (event == "exit_insufficient") {
            sndNoExit.play();
            td::String message;
            message.format("You only had %d gold (need 20).\nThe dungeon will reset - try again!", value);
            gui::Alert::show("Cannot Generate New Dungeon", message);
            gameState.resetPlayerPosition();
            reDraw();
        }
    }

    void processPendingMine() {
        if (!pendingMine.pending) return;

        int value = pendingMine.value;
        pendingMine.pending = false;
        pendingMine.value = 0;

        DialogLogin* dlg = DialogLogin::createWithRandomQuestion(this);
        dlg->openModal([this, value](gui::Dialog* pDlg) {
            DialogLogin* q = (DialogLogin*)pDlg;
            pendingMineResult.pending = true;
            pendingMineResult.wasCorrect = q->isAnswerCorrect();
            pendingMineResult.value = value;
            reDraw();
            });
    }

    void processPendingMineResult() {
        if (!pendingMineResult.pending) return;

        bool correct = pendingMineResult.wasCorrect;
        int  value = pendingMineResult.value;
        pendingMineResult.pending = false;

        td::String message;
        if (correct) {
            message.format("Correct! You avoid losing %d gold!", value);
            gui::Alert::show("Mine Avoided!", message);
        }
        else {
            gameState.applyMinePenalty();
            message.format("Wrong answer! You lose %d gold.\nRemaining: %d",
                value, gameState.getGold());
            gui::Alert::show("Mine Hit!", message);
        }
        reDraw();
    }

    void drawGameGrid() {
        int N = GameState::GRID_SIZE;
        gui::CoordType margin = leftZoneWidth * 0.01;
        gui::CoordType gridSize = leftZoneWidth - 2 * margin;
        gui::CoordType cellSize = gridSize / N;
        gui::CoordType startX = leftZoneLeft + margin;
        gui::CoordType startY = leftZoneTop + margin;

        if (backgroundLoaded) {
            try { imgBackground.draw(gui::Rect(startX, startY, startX + gridSize, startY + gridSize)); }
            catch (...) { backgroundLoaded = false; }
        }
        if (!backgroundLoaded) {
            gui::Shape gridBg;
            gridBg.createRect(gui::Rect(startX, startY, startX + gridSize, startY + gridSize));
            gridBg.drawFill(td::ColorID::DarkGray);
        }

        for (int i = 0; i <= N; i++) {
            gui::Point pts1[2] = { {startX + i * cellSize, startY}, {startX + i * cellSize, startY + gridSize} };
            gui::Shape line1; line1.createLines(pts1, 2, 2); line1.drawWire(td::ColorID::Gray);
            gui::Point pts2[2] = { {startX, startY + i * cellSize}, {startX + gridSize, startY + i * cellSize} };
            gui::Shape line2; line2.createLines(pts2, 2, 2); line2.drawWire(td::ColorID::Gray);
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int cellType = algorithmRunning ? displayGrid[i][j] : gameState.getDisplayCell(i, j);
                if (cellType != GameState::EMPTY)
                    drawCellContent(startX + i * cellSize, startY + j * cellSize, cellSize, cellType);
            }
        }

        if (isAnimating) {
            gui::Shape border;
            border.createRect(gui::Rect(startX, startY, startX + gridSize, startY + gridSize));
            border.drawWire(td::ColorID::Yellow, 3.0f);
        }
    }

    void drawCellContent(gui::CoordType x, gui::CoordType y, gui::CoordType size, int cellType) {
        gui::CoordType m = size * 0.1;
        gui::Rect cellRect(x + m, y + m, x + size - m, y + size - m);

        if (cellType == GameState::EXPLORED_NODE && showExploredNodes) {
            gui::Shape s1; s1.createRect(cellRect); s1.drawFill(td::ColorID::LightBlue);
            gui::Shape s2; s2.createRect(gui::Rect(x + m + 2, y + m + 2, x + size - m - 2, y + size - m - 2)); s2.drawFill(td::ColorID::SkyBlue);
            return;
        }

        if (cellType == GameState::PATH_VISUAL) {
            gui::CoordType gw = size * 0.08;
            gui::Shape g1; g1.createRoundedRect(gui::Rect(x + m - gw, y + m - gw, x + size - m + gw, y + size - m + gw), 4); g1.drawWire(td::ColorID::Yellow, gw);
            gui::Shape g2; g2.createRoundedRect(gui::Rect(x + m - gw * 0.5f, y + m - gw * 0.5f, x + size - m + gw * 0.5f, y + size - m + gw * 0.5f), 3); g2.drawWire(td::ColorID::Orange, gw * 0.7f);
            gui::Shape g3; g3.createRoundedRect(cellRect, 2); g3.drawWire(td::ColorID::White, 3);
            gui::Shape g4; g4.createRect(gui::Rect(x + m + 3, y + m + 3, x + size - m - 3, y + size - m - 3)); g4.drawFill(td::ColorID::LightYellow);
            return;
        }

        if (imagesLoaded) {
            try {
                if (cellType == GameState::PLAYER) { imgPlayer.draw(cellRect); return; }
                if (cellType == GameState::REWARD) { imgReward.draw(cellRect); return; }
                if (cellType == GameState::BANDIT) { imgBandit.draw(cellRect); return; }
                if (cellType == GameState::MINE) { imgMine.draw(cellRect);   return; }
                if (cellType == GameState::EXIT) { imgExit.draw(cellRect);   return; }
            }
            catch (...) { imagesLoaded = false; }
        }

        gui::Shape shape; shape.createRect(cellRect);
        if (cellType == GameState::PLAYER) shape.drawFill(td::ColorID::Green);
        if (cellType == GameState::REWARD) shape.drawFill(td::ColorID::Yellow);
        if (cellType == GameState::BANDIT) shape.drawFill(td::ColorID::Blue);
        if (cellType == GameState::MINE)   shape.drawFill(td::ColorID::Red);
        if (cellType == GameState::EXIT)   shape.drawFill(td::ColorID::White);
    }

    void drawControlPanel() {
        gui::CoordType x = rightZoneLeft;
        gui::CoordType y = rightZoneTop + 15;
        gui::CoordType w = rightZoneWidth;

        gui::CoordType labelW = w * 0.6;
        gui::CoordType speedW = w * 0.31;
        gui::DrawableString::draw("Select Algorithm:", 18,
            gui::Rect(x, y, x + labelW, y + 30),
            gui::Font::ID::SystemNormal, td::ColorID::White, td::TextAlignment::Left, td::VAlignment::Center);
        drawSpeedControl(x + labelW + w * 0.09, y, speedW);
        y += 30;

        gui::CoordType dropdownY = y;
        y += 70;

        drawStatisticsPanel(x, y, w);
        y += 165;

        drawControlButtons(x, y, w);
        y += 190;

        drawComparisonTable(x, y, w);
        drawAlgorithmDropdown(x, dropdownY, w);
    }

    void drawSpeedControl(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        speedButtonRect = gui::Rect(x, y, x + width, y + 24);
        gui::Shape bg; bg.createRoundedRect(speedButtonRect, 6);
        bg.drawFill(td::ColorID::Moss); bg.drawWire(td::ColorID::Copper, 2);

        char text[64];
        snprintf(text, sizeof(text), "Speed: %d ms  %s", animationSpeed, speedControlExpanded ? "🢐" : "🢒");
        gui::DrawableString::draw(text, strlen(text), speedButtonRect,
            gui::Font::ID::SystemSmaller, td::ColorID::White, td::TextAlignment::Center, td::VAlignment::Center);

        if (speedControlExpanded)
            drawSpeedSlider(x + width + 15, y - 15, 11, 120);
    }

    void drawSpeedSlider(gui::CoordType x, gui::CoordType y, gui::CoordType w, gui::CoordType h) {
        speedSliderRect = gui::Rect(x, y, x + w, y + h);
        gui::Shape track; track.createRoundedRect(speedSliderRect, 4);
        track.drawFill(td::ColorID::DarkGray); track.drawWire(td::ColorID::Copper, 1);

        gui::CoordType fillH = (std::min(MAX_SPEED, animationSpeed) * h) / MAX_SPEED;
        if (fillH > 0) {
            gui::Shape fill; fill.createRoundedRect(gui::Rect(x, y, x + w, y + fillH), 4);
            fill.drawFill(td::ColorID::Orange);
        }

        gui::CoordType handleY = y + fillH;
        gui::Shape handle; handle.createRect(gui::Rect(x - 3, handleY - 6, x + w + 3, handleY + 6));
        handle.drawFill(td::ColorID::White); handle.drawWire(td::ColorID::Copper, 2);

        gui::DrawableString::draw("Fast", 4, gui::Rect(x - 15, y - 25, x + w + 15, y - 3),
            gui::Font::ID::SystemSmaller, td::ColorID::LightGray, td::TextAlignment::Center, td::VAlignment::Bottom);
        gui::DrawableString::draw("Slow", 4, gui::Rect(x - 15, y + h + 5, x + w + 15, y + h + 25),
            gui::Font::ID::SystemSmaller, td::ColorID::LightGray, td::TextAlignment::Center, td::VAlignment::Top);
    }

    void handleSpeedSliderClick(const gui::Point& click) {
        gui::CoordType h = speedSliderRect.height();
        gui::CoordType cy = std::max<gui::CoordType>(0, std::min(click.y - speedSliderRect.top, h));
        animationSpeed = std::max(10, (int)((cy * MAX_SPEED) / h));
        reDraw();
    }

    void drawAlgorithmDropdown(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        dropdownRect = gui::Rect(x, y, x + width, y + 50);
        gui::Shape bg; bg.createRoundedRect(dropdownRect, 6); bg.drawFill(td::ColorID::Moss);
        gui::Shape border; border.createRoundedRect(dropdownRect, 6); border.drawWire(td::ColorID::LightGreen, 2);

        const char* names[] = { "Select Algorithm...", "Breadth-First Search (BFS)",
            "Depth-First Search (DFS)", "Dijkstra Search",
            "A* Search", "Greedy Best-First Search", "MDP (Markov Decision Process)" };
        std::string label = names[currentAlgorithm];

        gui::DrawableString::draw(label.c_str(), label.length(),
            gui::Rect(x + 15, y, x + width - 40, y + 50),
            gui::Font::ID::SystemNormal, td::ColorID::White, td::TextAlignment::Left, td::VAlignment::Center);

        const char* arrow = dropdownExpanded ? "^" : "v";
        gui::DrawableString::draw(arrow, 1, gui::Rect(x + width - 35, y, x + width - 10, y + 50),
            gui::Font::ID::SystemBold, td::ColorID::White, td::TextAlignment::Center, td::VAlignment::Center);

        if (dropdownExpanded) {
            gui::CoordType menuY = y + 53;
            const char* options[] = { "Breadth-First Search (BFS)", "Depth-First Search (DFS)",
                "Dijkstra Search", "A* Search",
                "Greedy Best-First Search", "MDP (Markov Decision Process)" };

            gui::CoordType itemH = 45;
            gui::Shape menuBg; menuBg.createRoundedRect(gui::Rect(x, menuY, x + width, menuY + 6 * itemH), 6);
            menuBg.drawFill(td::ColorID::Moss);
            gui::Shape menuBorder; menuBorder.createRoundedRect(gui::Rect(x, menuY, x + width, menuY + 6 * itemH), 6);
            menuBorder.drawWire(td::ColorID::LightGreen, 2);

            for (int i = 0; i < 6; i++) {
                gui::CoordType iy = menuY + i * itemH;
                dropdownItemRects[i] = gui::Rect(x, iy, x + width, iy + itemH);
                if (i + 1 == currentAlgorithm) {
                    gui::Shape hi; hi.createRect(gui::Rect(x + 3, iy + 2, x + width - 3, iy + itemH - 2));
                    hi.drawFill(td::ColorID::DarkRed);
                }
                gui::DrawableString::draw(options[i], strlen(options[i]),
                    gui::Rect(x + 15, iy, x + width - 15, iy + itemH),
                    gui::Font::ID::SystemNormal, td::ColorID::White, td::TextAlignment::Left, td::VAlignment::Center);
            }
        }
    }

    void drawStatisticsPanel(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::Shape bg; bg.createRoundedRect(gui::Rect(x, y, x + width, y + 150), 6); bg.drawFill(td::ColorID::Moss);
        gui::Shape border; border.createRoundedRect(gui::Rect(x, y, x + width, y + 150), 6); border.drawWire(td::ColorID::LightGreen, 2);

        gui::CoordType cy = y + 20;
        gui::CoordType hw = (width - 40) / 2;

        std::string status;
        if (isAnimating)                 status = "Animating";
        else if (algorithmRunning)       status = "Paused";
        else if (gameState.isGameOver()) status = gameState.isGameWon() ? "Reached the Exit!" : "Game Over";
        else                             status = "Playing";

        std::string gold = std::to_string(gameState.getGold());

        gui::DrawableString::draw("Current Gold", 12, gui::Rect(x + 20, cy, x + 20 + hw - 15, cy + 22), gui::Font::ID::SystemNormal, td::ColorID::LightGray, td::TextAlignment::Left, td::VAlignment::Center);
        gui::DrawableString::draw(gold.c_str(), gold.length(), gui::Rect(x + 20, cy + 25, x + 20 + hw - 15, cy + 50), gui::Font::ID::SystemBold, td::ColorID::Yellow, td::TextAlignment::Left, td::VAlignment::Center);
        gui::DrawableString::draw("Status", 6, gui::Rect(x + 20 + hw + 15, cy, x + width - 20, cy + 22), gui::Font::ID::SystemNormal, td::ColorID::LightGray, td::TextAlignment::Right, td::VAlignment::Center);
        gui::DrawableString::draw(status.c_str(), status.length(), gui::Rect(x + 20 + hw + 15, cy + 25, x + width - 20, cy + 50), gui::Font::ID::SystemBold, td::ColorID::LightGreen, td::TextAlignment::Right, td::VAlignment::Center);
        cy += 65;

        std::string pathStr = algorithmRunning ? std::to_string(currentPathIndex) + "/" + std::to_string(fullAlgorithmPath.size()) : "0";
        std::string explStr = algorithmRunning ? std::to_string(currentExploredIndex) + "/" + std::to_string(fullExploredNodes.size()) : "0";

        gui::DrawableString::draw("Path Progress", 13, gui::Rect(x + 20, cy, x + 20 + hw - 15, cy + 22), gui::Font::ID::SystemNormal, td::ColorID::LightGray, td::TextAlignment::Left, td::VAlignment::Center);
        gui::DrawableString::draw(pathStr.c_str(), pathStr.length(), gui::Rect(x + 20, cy + 25, x + 20 + hw - 15, cy + 50), gui::Font::ID::SystemBold, td::ColorID::Yellow, td::TextAlignment::Left, td::VAlignment::Center);
        gui::DrawableString::draw("Explored Nodes", 14, gui::Rect(x + 20 + hw + 15, cy, x + width - 20, cy + 22), gui::Font::ID::SystemNormal, td::ColorID::LightGray, td::TextAlignment::Right, td::VAlignment::Center);
        gui::DrawableString::draw(explStr.c_str(), explStr.length(), gui::Rect(x + 20 + hw + 15, cy + 25, x + width - 20, cy + 50), gui::Font::ID::SystemBold, td::ColorID::LightGreen, td::TextAlignment::Right, td::VAlignment::Center);
    }

    void drawControlButtons(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::CoordType bh = 45, gap = 12, hw = width / 2 - 6;

        startButtonRect = gui::Rect(x, y, x + hw, y + bh);
        pauseButtonRect = gui::Rect(x + hw + 12, y, x + width, y + bh);
        drawButton("START", x, y, hw, bh, td::ColorID::Moss, algorithmRunning && !isAnimating);
        drawButton("PAUSE", x + hw + 12, y, hw, bh, td::ColorID::Moss, algorithmRunning && isAnimating);
        y += bh + gap;

        stepButtonRect = gui::Rect(x, y, x + hw, y + bh);
        resetButtonRect = gui::Rect(x + hw + 12, y, x + width, y + bh);
        drawButton("STEP", x, y, hw, bh, td::ColorID::Moss, algorithmRunning && !isAnimating);
        drawButton("RESET", x + hw + 12, y, hw, bh, td::ColorID::Moss, algorithmRunning);
        y += bh + gap;

        generateNewGameRect = gui::Rect(x, y, x + width, y + bh);
        drawButton("GENERATE NEW DUNGEON", x, y, width, bh, td::ColorID::Copper, true);
    }

    void drawButton(const char* label, gui::CoordType x, gui::CoordType y, gui::CoordType w, gui::CoordType h, td::ColorID color, bool enabled) {
        gui::Rect r(x, y, x + w, y + h);
        gui::Shape bg; bg.createRoundedRect(r, 6); bg.drawFill(enabled ? color : td::ColorID::DimGray);
        gui::Shape border; border.createRoundedRect(r, 6); border.drawWire(enabled ? td::ColorID::Gray : td::ColorID::DarkGray, 1);
        gui::DrawableString::draw(label, strlen(label), r, gui::Font::ID::SystemNormal, td::ColorID::White, td::TextAlignment::Center, td::VAlignment::Center);
    }

    void drawComparisonTable(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        gui::DrawableString::draw("Algorithm Comparison", 21,
            gui::Rect(x, y, x + width, y + 30),
            gui::Font::ID::SystemNormal, td::ColorID::White, td::TextAlignment::Left, td::VAlignment::Center);
        y += 35;

        gui::CoordType tableH = currentAlgorithm > 0 ? 200 : 80;
        gui::Shape bg; bg.createRoundedRect(gui::Rect(x, y, x + width, y + tableH), 6); bg.drawFill(td::ColorID::Moss);
        gui::Shape border; border.createRoundedRect(gui::Rect(x, y, x + width, y + tableH), 6); border.drawWire(td::ColorID::LightGreen, 2);

        if (currentAlgorithm > 0)
            drawAlgorithmDetails(x + 15, y + 15, width - 10);
        else {
            const char* msg = "Select and run an algorithm to see details";
            gui::DrawableString::draw(msg, strlen(msg),
                gui::Rect(x + 20, y + 30, x + width - 20, y + 60),
                gui::Font::ID::SystemNormal, td::ColorID::LightGray, td::TextAlignment::Center, td::VAlignment::Center);
        }
    }

    void drawAlgorithmDetails(gui::CoordType x, gui::CoordType y, gui::CoordType width) {
        const char* name = "", * desc = "", * heuristic = "", * timeC = "", * spaceC = "";

        if (currentAlgorithm == 1) {
            name = "BFS (Breadth-First Search)";
            desc = "Explores level-by-level using queue. Guarantees shortest path (unweighted).";
            heuristic = "None (blind search)"; timeC = "O(V + E) = O(b^d)"; spaceC = "O(V) = O(b^d)";
        }
        else if (currentAlgorithm == 2) {
            name = "DFS (Depth-First Search)";
            desc = "Explores depth-first using stack. Does NOT guarantee shortest path.";
            heuristic = "None (blind search)"; timeC = "O(V + E) = O(b^d)"; spaceC = "O(h) = O(bd) if limited";
        }
        else if (currentAlgorithm == 3) {
            name = "Dijkstra";
            desc = "Uses priority queue for min cost. Costs: Rewards=0, Mines=8, Bandits=15.";
            heuristic = "None (uses actual cost only)"; timeC = "O((V + E) log V)"; spaceC = "O(V)";
        }
        else if (currentAlgorithm == 4) {
            name = "A* Search";
            desc = "Combines cost (g) + heuristic (h). Optimal if heuristic is admissible.";
            heuristic = "Manhattan distance: |x1-x2| + |y1-y2|"; timeC = "O((V + E) log V)"; spaceC = "O(V)";
        }
        else if (currentAlgorithm == 5) {
            name = "Greedy Best-First";
            desc = "Uses ONLY heuristic to goal, ignores cost. Fast but NOT optimal.";
            heuristic = "Manhattan distance: |x1-x2| + |y1-y2|"; timeC = "O(b^d) worst case"; spaceC = "O(b^d)";
        }
        else if (currentAlgorithm == 6) {
            name = "MDP (Markov Decision Process)";
            desc = "Finds optimal policy considering uncertainty (mine questions 70% success).";
            heuristic = "Value Iteration with Bellman Equation"; timeC = "O(|S| * |A| * iterations)"; spaceC = "O(|S|) where S = states";
        }

        gui::CoordType lh = 20, cy = y;
        gui::DrawableString::draw(name, strlen(name), gui::Rect(x, cy, x + width, cy + lh + 2), gui::Font::ID::SystemBold, td::ColorID::Yellow, td::TextAlignment::Left, td::VAlignment::Top);
        cy += lh + 8;
        gui::DrawableString::draw(desc, strlen(desc), gui::Rect(x, cy, x + width, cy + lh * 3), gui::Font::ID::SystemSmaller, td::ColorID::LightGray, td::TextAlignment::Left, td::VAlignment::Top);
        cy += lh * 2 + 12;

        char buf[128];
        snprintf(buf, sizeof(buf), "Heuristic: %s", heuristic);
        gui::DrawableString::draw(buf, strlen(buf), gui::Rect(x, cy, x + width, cy + lh), gui::Font::ID::SystemSmaller, td::ColorID::White, td::TextAlignment::Left, td::VAlignment::Top);
        cy += lh + 6;
        snprintf(buf, sizeof(buf), "Time Complexity: %s", timeC);
        gui::DrawableString::draw(buf, strlen(buf), gui::Rect(x, cy, x + width, cy + lh), gui::Font::ID::SystemSmaller, td::ColorID::LightGreen, td::TextAlignment::Left, td::VAlignment::Top);
        cy += lh + 6;
        snprintf(buf, sizeof(buf), "Space Complexity: %s", spaceC);
        gui::DrawableString::draw(buf, strlen(buf), gui::Rect(x, cy, x + width, cy + lh), gui::Font::ID::SystemSmaller, td::ColorID::LightGreen, td::TextAlignment::Left, td::VAlignment::Top);
        cy += lh + 6;
        snprintf(buf, sizeof(buf), "Execution Time: %.3f ms", algorithmExecTimeUs / 1000.0);
        gui::DrawableString::draw(buf, strlen(buf), gui::Rect(x, cy, x + width, cy + lh), gui::Font::ID::SystemSmaller, td::ColorID::Cyan, td::TextAlignment::Left, td::VAlignment::Top);
    }

    void playSoundtrack() {
        if (!gameState.isGameOver() && !algorithmRunning && !soundtrackPlaying) {
            sndSoundtrack.play();
            soundtrackPlaying = true;
        }
    }

protected:
    bool onKeyPressed(const gui::Key& key) override {
        if (key.isVirtual()) {
            gui::Key::Virtual k = key.getVirtual();
            if (k == gui::Key::Virtual::Right) {
                if (gameState.movePlayer(gameState.getPlayerX() + 1, gameState.getPlayerY())) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (k == gui::Key::Virtual::Left) {
                if (gameState.movePlayer(gameState.getPlayerX() - 1, gameState.getPlayerY())) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (k == gui::Key::Virtual::Up) {
                if (gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() - 1)) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (k == gui::Key::Virtual::Down) {
                if (gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() + 1)) { playSoundtrack(); reDraw(); }
                return true;
            }
        }

        if (key.isASCII()) {
            char ch = key.getChar();
            if (ch == 'w' || ch == 'W') {
                if (gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() - 1)) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (ch == 's' || ch == 'S') {
                if (gameState.movePlayer(gameState.getPlayerX(), gameState.getPlayerY() + 1)) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (ch == 'a' || ch == 'A') {
                if (gameState.movePlayer(gameState.getPlayerX() - 1, gameState.getPlayerY())) { playSoundtrack(); reDraw(); }
                return true;
            }
            if (ch == 'd' || ch == 'D') {
                if (gameState.movePlayer(gameState.getPlayerX() + 1, gameState.getPlayerY())) { playSoundtrack(); reDraw(); }
                return true;
            }
        }

        return gui::Canvas::onKeyPressed(key);
    }

    void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override {
        gui::Point click = inputDevice.getModelPoint();

        if (speedButtonRect.contains(click)) { speedControlExpanded = !speedControlExpanded; reDraw(); return; }
        if (speedControlExpanded && speedSliderRect.contains(click)) { handleSpeedSliderClick(click); return; }
        if (dropdownRect.contains(click)) { dropdownExpanded = !dropdownExpanded; reDraw(); return; }

        if (dropdownExpanded) {
            for (int i = 0; i < 6; i++) {
                if (dropdownItemRects[i].contains(click)) {
                    currentAlgorithm = i + 1;
                    dropdownExpanded = false;
                    if (gameState.isGameOver())
                        runAlgorithm(static_cast<AlgorithmType>(currentAlgorithm));
                    reDraw();
                    return;
                }
            }
            dropdownExpanded = false;
            reDraw();
            return;
        }

        if (startButtonRect.contains(click) && algorithmRunning) { startAnimation(); return; }
        if (pauseButtonRect.contains(click) && algorithmRunning) { pauseAnimation(); return; }
        if (stepButtonRect.contains(click) && algorithmRunning) { stepAnimation();  return; }
        if (resetButtonRect.contains(click) && algorithmRunning) { resetAlgorithmVisualization(); return; }
        if (generateNewGameRect.contains(click)) { resetGame(); return; }
    }

    void onResize(const gui::Size& newSize) override {
        gui::CoordType minDim = std::min(newSize.width, newSize.height);
        leftZoneWidth = minDim * 0.9;
        leftZoneLeft = newSize.width * 0.03;
        leftZoneTop = (newSize.height - leftZoneWidth) / 2;
        gui::CoordType gap = newSize.width * 0.015;
        rightZoneLeft = leftZoneLeft + leftZoneWidth + gap;
        rightZoneWidth = newSize.width - rightZoneLeft - (newSize.width * 0.03);
        rightZoneTop = newSize.height * 0.05;
        reDraw();
    }

    void onDraw(const gui::Rect& rect) override {
        processPendingMine();
        processPendingMineResult();

        if (algorithmRunning && isAnimating) updateAnimation();
        if (algorithmRunning) updateVisualization();

        gui::Shape bg;
        bg.createRect(rect);
        bg.drawFill(td::ColorID::Moss);

        drawGameGrid();
        drawControlPanel();
    }

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
        , sndReward(":rewardSound")
        , sndMine(":mineSound")
        , sndBandit(":banditSound")
        , sndExit(":exitSound")
        , sndNoExit(":noExitSound")
        , sndSoundtrack(":soundtrack") {

        enableResizeEvent(true);
        lastAnimationTime = std::chrono::steady_clock::now();

        gameState.setGameEventCallback([this](const std::string& event, int value) {
            handleGameEvent(event, value);
            });
    }

    ~SimulationCanvas() = default;

    bool isGameOver() const { return gameState.isGameOver(); }
    bool isGameWon()  const { return gameState.isGameWon(); }
    int  getGold()    const { return gameState.getGold(); }

    std::pair<int, int> getPlayerPosition() const {
        return { gameState.getPlayerX(), gameState.getPlayerY() };
    }

    void setAnimationSpeed(int speedMS) { animationSpeed = speedMS; }
    int  getAnimationSpeed() const { return animationSpeed; }

    void runAlgorithm(AlgorithmType type) {
        if (!gameState.isGameOver()) return;

        algorithmRunning = true;
        isAnimating = (type != AlgorithmType::BFS);

        const auto& initialState = gameState.getInitialState();
        std::pair<int, int> start = { initialState.playerStartX, initialState.playerStartY };
        std::pair<int, int> exit = { initialState.exitX,        initialState.exitY };

        auto searchStart = std::chrono::steady_clock::now();

        DungeonAlgorithms::SearchResult result;
        if (type == AlgorithmType::BFS)    result = DungeonAlgorithms::bfsSearch(initialState.actualGrid, start, exit);
        else if (type == AlgorithmType::DFS)    result = DungeonAlgorithms::dfsSearch(initialState.actualGrid, start, exit);
        else if (type == AlgorithmType::DIJKSTRA)    result = DungeonAlgorithms::dijkstraSearch(initialState.actualGrid, start, exit);
        else if (type == AlgorithmType::AStar)  result = DungeonAlgorithms::aStarSearch(initialState.actualGrid, start, exit);
        else if (type == AlgorithmType::Greedy) result = DungeonAlgorithms::greedySearch(initialState.actualGrid, start, exit);
        else if (type == AlgorithmType::MDP)    result = DungeonAlgorithms::mdpSearch(initialState.actualGrid, start, exit, gameState.getGold());

        auto searchEnd = std::chrono::steady_clock::now();
        algorithmExecTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(searchEnd - searchStart).count();

        fullAlgorithmPath = result.path;
        fullExploredNodes = result.exploredNodes;
        currentAlgorithm = static_cast<int>(type);
        lastAnimationTime = std::chrono::steady_clock::now();
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;

        setupAlgorithmVisualization();
    }

    void startAnimation() {
        if (!algorithmRunning) return;
        isAnimating = true;
        lastAnimationTime = std::chrono::steady_clock::now();
        gui::Canvas::startAnimation();
        reDraw();
    }

    void pauseAnimation() {
        if (!algorithmRunning) return;
        isAnimating = false;
        gui::Canvas::stopAnimation();
        reDraw();
    }

    void stepAnimation() {
        if (!algorithmRunning) return;
        if (animationPhase == 0) {
            if (currentExploredIndex < (int)fullExploredNodes.size()) currentExploredIndex++;
            else animationPhase = 1;
        }
        else if (animationPhase == 1) {
            if (currentPathIndex < (int)fullAlgorithmPath.size()) currentPathIndex++;
        }
        reDraw();
    }

    void updateAnimation() {
        if (!isAnimating || !algorithmRunning) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAnimationTime).count();
        if (elapsed < animationSpeed) return;

        if (animationPhase == 0) {
            if (currentExploredIndex < (int)fullExploredNodes.size()) {
                currentExploredIndex++;
                lastAnimationTime = now;
            }
            else { animationPhase = 1; lastAnimationTime = now; }
        }
        else if (animationPhase == 1) {
            if (currentPathIndex < (int)fullAlgorithmPath.size()) {
                currentPathIndex++;
                lastAnimationTime = now;
            }
            else { isAnimating = false; gui::Canvas::stopAnimation(); }
        }
    }

    void resetAlgorithmVisualization() {
        algorithmRunning = false;
        isAnimating = false;
        currentAlgorithm = 0;
        algorithmExecTimeUs = 0;
        fullAlgorithmPath.clear();
        fullExploredNodes.clear();
        currentExploredIndex = 0;
        currentPathIndex = 0;
        animationPhase = 0;
        gameState.resetVisualization();
        reDraw();
    }

    void toggleExploredNodes() {
        showExploredNodes = !showExploredNodes;
        reDraw();
    }
};
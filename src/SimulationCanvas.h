#pragma once
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Image.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>

class MainView; // Forward declaration

class SimulationCanvas : public gui::Canvas {
private:
    gui::CoordType gridLeft = 0;
    gui::CoordType gridTop = 0;
    gui::CoordType gridSize = 0;

    std::mt19937 rng;

    MainView* _pParent = nullptr;

    gui::Image imgPlayer;
    gui::Image imgReward;
    gui::Image imgBandit;
    gui::Image imgMine;
    gui::Image imgExit;
    gui::Image imgBackground;

    bool imagesLoaded = true;
    bool backgroundLoaded = true;

    struct GameState {
        int playerX = 0;
        int playerY = 0;
        int gold = 0;
        bool gameOver = false;
        bool gameWon = false;

        int grid[10][10] = { {0} };

        GameState(std::mt19937& rng) : playerX(0), playerY(0), gold(0), gameOver(false), gameWon(false) {
            memset(grid, 0, sizeof(grid));
            std::uniform_int_distribution<int> rowDist(0, 9);
            std::uniform_int_distribution<int> colDist(0, 9);
            std::uniform_int_distribution<int> playerRowDist(0, 9);

            playerX = 0;
            playerY = playerRowDist(rng);
            grid[playerX][playerY] = 1;

            int exitRow = rowDist(rng);
            grid[9][exitRow] = 5;

            placeRandomTile(rng, grid, 2, 2);  // 2 rewards
            placeRandomTile(rng, grid, 1, 3);  // 1 bandit
            placeRandomTile(rng, grid, 2, 4);  // 2 mines
        }

        static void placeRandomTile(std::mt19937& rng, int grid[10][10], int count, int tileType) {
            std::uniform_int_distribution<int> rowDist(0, 9);
            std::uniform_int_distribution<int> colDist(0, 9);

            for (int i = 0; i < count; i++) {
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
                }
            }
        }

        bool movePlayer(int newX, int newY, bool& hitMine) {
            hitMine = false;

            if (gameOver || newX < 0 || newX >= 10 || newY < 0 || newY >= 10)
                return false;

            int cellType = grid[newX][newY];

            grid[playerX][playerY] = 0;
            playerX = newX;
            playerY = newY;

            if (cellType == 2) {  // Reward
                gold += 10;
                grid[newX][newY] = 1;
                return true;
            }
            else if (cellType == 3) {  // Bandit
                gold = gold / 2;
                grid[newX][newY] = 1;
                return true;
            }
            else if (cellType == 4) {  // Mine - trigger quiz
                hitMine = true;
                // Don't change the grid yet, wait for quiz result
                return true;
            }
            else if (cellType == 5) {  // Exit
                grid[newX][newY] = 1;
                gameOver = true;
                gameWon = true;
                return true;
            }
            else {
                grid[newX][newY] = 1;
                return true;
            }
        }

        void resolveMineTile(bool answeredCorrectly) {
            if (answeredCorrectly) {
                // Player survives, just mark the tile as visited
                grid[playerX][playerY] = 1;
            }
            else {
                // Game over
                gameOver = true;
                gameWon = false;
            }
        }
    };

    GameState gameState;

    bool _waitingForQuizResult = false;

public:
    SimulationCanvas()
        : gui::Canvas({ gui::InputDevice::Event::Keyboard })
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
        setFocus();
    }

    ~SimulationCanvas() = default;

    void setParent(MainView* p) {
        _pParent = p;
    }

    // Declare these methods - implementations will be in MainView.cpp
    // because they call methods on MainView* which is an incomplete type here
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void showQuiz();
    void handleQuizResult(bool answeredCorrectly);

    void resetGame() {
        gameState = GameState(rng);
        reDraw();
    }

    bool isGameOver() const {
        return gameState.gameOver;
    }

    bool isGameWon() const {
        return gameState.gameWon;
    }

    int getGold() const {
        return gameState.gold;
    }

    std::pair<int, int> getPlayerPosition() const {
        return { gameState.playerX, gameState.playerY };
    }

protected:
    void onResize(const gui::Size& newSize) override {
        gui::CoordType minDimension = std::min(newSize.width, newSize.height);
        gridSize = minDimension * 0.9;
        gridLeft = (newSize.width - gridSize) / 2;
        gridTop = (newSize.height - gridSize) / 2;
        this->reDraw();
    }

    bool onKeyPressed(const gui::Key& key) override
    {
        if (gameState.gameOver)
            return true;

        if (key.isVirtual())
        {
            auto virtualKey = key.getVirtual();
            if (virtualKey == gui::Key::Virtual::Up) {
                moveUp();
                return true;
            }
            else if (virtualKey == gui::Key::Virtual::Down) {
                moveDown();
                return true;
            }
            else if (virtualKey == gui::Key::Virtual::Left) {
                moveLeft();
                return true;
            }
            else if (virtualKey == gui::Key::Virtual::Right) {
                moveRight();
                return true;
            }
        }
        else
        {
            char c = key.getChar();
            if (c == 'w' || c == 'W') {
                moveUp();
                return true;
            }
            if (c == 's' || c == 'S') {
                moveDown();
                return true;
            }
            if (c == 'a' || c == 'A') {
                moveLeft();
                return true;
            }
            if (c == 'd' || c == 'D') {
                moveRight();
                return true;
            }
        }

        return gui::Canvas::onKeyPressed(key);
    }

    void onDraw(const gui::Rect& rect) override {
        gui::Shape bg;
        bg.createRect(rect);
        bg.drawFill(td::ColorID::Black);
        drawGameGrid();
    }

private:
    void drawGameGrid() {
        const int GRID_SIZE = 10;

        gui::CoordType margin = gridSize * 0.01;
        gui::CoordType gridAreaSize = gridSize - 2 * margin;
        gui::CoordType cellSize = gridAreaSize / GRID_SIZE;

        gui::CoordType gridStartX = gridLeft + margin;
        gui::CoordType gridStartY = gridTop + margin;

        if (backgroundLoaded) {
            imgBackground.draw(gui::Rect(gridStartX, gridStartY,
                gridStartX + gridAreaSize,
                gridStartY + gridAreaSize));
        }
        else {
            gui::Shape gridBg;
            gridBg.createRect(gui::Rect(gridStartX, gridStartY,
                gridStartX + gridAreaSize,
                gridStartY + gridAreaSize));
            gridBg.drawFill(td::ColorID::Black);
        }

        for (int j = 0; j <= GRID_SIZE; j++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = { gridStartX, gridStartY + j * cellSize };
            points[1] = { gridStartX + gridAreaSize, gridStartY + j * cellSize };
            line.createLines(points, 2, 5);
            line.drawWire(td::ColorID::Black);
        }

        for (int i = 0; i <= GRID_SIZE; i++) {
            gui::Shape line;
            gui::Point points[2];
            points[0] = { gridStartX + i * cellSize, gridStartY };
            points[1] = { gridStartX + i * cellSize, gridStartY + gridAreaSize };
            line.createLines(points, 2, 5);
            line.drawWire(td::ColorID::Black);
        }

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
        gui::CoordType margin = size * 0.1;
        gui::Rect cellRect(x + margin, y + margin,
            x + size - margin, y + size - margin);

        if (imagesLoaded) {
            try {
                switch (cellType) {
                case 1: imgPlayer.draw(cellRect); break;
                case 2: imgReward.draw(cellRect); break;
                case 3: imgBandit.draw(cellRect); break;
                case 4: imgMine.draw(cellRect); break;
                case 5: imgExit.draw(cellRect); break;
                }
                return;
            }
            catch (...) {
            }
        }

        gui::Shape cellShape;
        cellShape.createRect(cellRect);

        switch (cellType) {
        case 1: cellShape.drawFill(td::ColorID::Green); break;
        case 2: cellShape.drawFill(td::ColorID::Yellow); break;
        case 3: cellShape.drawFill(td::ColorID::Blue); break;
        case 4: cellShape.drawFill(td::ColorID::Red); break;
        case 5: cellShape.drawFill(td::ColorID::White); break;
        }
    }
};
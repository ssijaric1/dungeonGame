#pragma once
#include <gui/Canvas.h>
#include <gui/Image.h>
#include <gui/Transformation.h>
#include <math/math.h>
#include <vector>
#include <time.h>
#include "StatusBar.h" 
#include <gui/natGUI.h> 

#define MAX_GRID_SIZE 32

class ViewDungeon : public gui::Canvas
{
public:
    enum class TileType {
        Empty = 0,
        Start,
        Exit,
        Reward,
        Bandit,
        Mine,
        Wall,
        Player,
        Visited,
        Path
    };

    enum class Algorithm { DFS = 0, BFS, UCS, Greedy, AStar };

protected:
    gui::Image _tileImages[10];
    StatusBar* _pStatusBar;

    td::INT4 _width, _height;
    td::INT4 _playerX, _playerY;
    td::INT4 _gold;
    td::INT4 _steps;

    TileType _grid[MAX_GRID_SIZE][MAX_GRID_SIZE];  // Logical grid
    TileType _sgrid[MAX_GRID_SIZE][MAX_GRID_SIZE]; // Display grid

    const gui::CoordType _tileSize = 32;
    gui::Rect _tileRect;

    bool _gameRunning = false;
    bool _gameWon = false;
    Algorithm _currentAlgorithm = Algorithm::DFS;

    // Algorithm visualization
    std::vector<std::pair<td::INT4, td::INT4>> _explorationPath;
    td::INT4 _currentStep = 0;
    bool _visualizationMode = false;

protected:
    void onDraw(const gui::Rect& rect) override
    {
        for (td::INT4 i = 0; i < _width; i++) {
            for (td::INT4 j = 0; j < _height; j++)
            {
                _tileRect = { i * _tileSize, j * _tileSize, _tileSize };
                gui::Transformation::saveContext();
                gui::Transformation tr;
                // Draw the image for the current tile type
                _tileImages[static_cast<int>(_sgrid[i][j])].draw(_tileRect);
                tr.restoreContext();
            }
        }
    }

    bool onKeyPressed(const gui::Key& key) override
    {
        if (!_gameRunning || _visualizationMode)
            return false;

        td::INT4 newX = _playerX;
        td::INT4 newY = _playerY;

        // Check if it's a virtual key
        if (!key.isVirtual())
        {
            // Check for WASD (ASCII keys)
            if (key.isASCII())
            {
                char c = key.getChar();
                if (c == 'w' || c == 'W')
                    newY--;
                else if (c == 's' || c == 'S')
                    newY++;
                else if (c == 'a' || c == 'A')
                    newX--;
                else if (c == 'd' || c == 'D')
                    newX++;
                else
                    return false; // Not handled
            }
            else
            {
                return false; // Not handled
            }
        }
        else
        {
            // Handle arrow keys (virtual keys)
            auto vKey = key.getVirtual();

            if (vKey == gui::Key::Virtual::Up)
                newY--;
            else if (vKey == gui::Key::Virtual::Down)
                newY++;
            else if (vKey == gui::Key::Virtual::Left)
                newX--;
            else if (vKey == gui::Key::Virtual::Right)
                newX++;
            else
                return false; // Not handled
        }

        // Check bounds
        if (newX < 0 || newX >= _width || newY < 0 || newY >= _height)
            return true; // Key handled, but invalid move

        // Check walls
        if (_grid[newX][newY] == TileType::Wall)
            return true; // Key handled, but hit wall

        // Move player
        movePlayer(newX, newY);
        reDraw();

        return true; // Key handled successfully
    }

    void movePlayer(td::INT4 newX, td::INT4 newY)
    {
        _sgrid[_playerX][_playerY] = _grid[_playerX][_playerY];
        _playerX = newX;
        _playerY = newY;
        _steps++;

        handleTileEffect(newX, newY);
        _sgrid[_playerX][_playerY] = TileType::Player;
        updateStatus();
    }

    void handleTileEffect(td::INT4 x, td::INT4 y)
    {
        switch (_grid[x][y])
        {
        case TileType::Reward:
            _gold += 10;
            _pStatusBar->setMessage("Found reward! +10 gold");
            _grid[x][y] = TileType::Empty;
            break;

        case TileType::Bandit:
            _gold = _gold / 2;
            _pStatusBar->setMessage("Bandit attacked! Lost 50% gold");
            _grid[x][y] = TileType::Empty;
            break;

        case TileType::Mine:
            _pStatusBar->setMessage("Mine triggered! Answer the quiz...");
            showMineQuiz(x, y);
            break;

        case TileType::Exit:
            _gameRunning = false;
            _gameWon = true;
            _pStatusBar->setMessage("You reached the exit! You win!");
            break;

        default:
            break;
        }
    }

    void showMineQuiz(td::INT4 x, td::INT4 y)
    {
        gui::Alert::showYesNoQuestion(
            "Mine Quiz",
            "What is 2+2?",
            "4",
            "5",
            [this, x, y](gui::Alert::Answer answer) {
                // This callback is executed when user clicks a button
                if (answer == gui::Alert::Answer::Yes) {
                    // User clicked "4" (correct answer)
                    _pStatusBar->setMessage("Quiz passed! Mine disarmed.");
                    _grid[x][y] = TileType::Empty;
                }
                else {
                    // User clicked "5" (wrong answer)
                    _pStatusBar->setMessage("Quiz failed! Lost 5 gold.");
                    _gold = std::max(0, _gold - 5);
                }

                // Redraw to show changes
                reDraw();
            }
        );
    }

    void updateStatus()
    {
        _pStatusBar->setNoOfGold(_gold);
        _pStatusBar->setNoOfSteps(_steps);
    }

    void initializeLevel()
    {
        for (td::INT4 i = 0; i < _width; i++) {
            for (td::INT4 j = 0; j < _height; j++) {
                _grid[i][j] = TileType::Empty;
                _sgrid[i][j] = TileType::Empty;
            }
        }

        _width = 12;
        _height = 10;

        for (td::INT4 i = 0; i < _width; i++) {
            _grid[i][0] = TileType::Wall;
            _grid[i][_height - 1] = TileType::Wall;
        }
        for (td::INT4 j = 0; j < _height; j++) {
            _grid[0][j] = TileType::Wall;
            _grid[_width - 1][j] = TileType::Wall;
        }

        _playerX = 1; _playerY = 1;
        _grid[1][1] = TileType::Start;
        _grid[_width - 2][_height - 2] = TileType::Exit;

        _grid[3][3] = TileType::Reward;
        _grid[5][5] = TileType::Bandit;
        _grid[7][7] = TileType::Mine;
        _grid[4][4] = TileType::Wall;
        _grid[4][5] = TileType::Wall;

        for (td::INT4 i = 0; i < _width; i++) {
            for (td::INT4 j = 0; j < _height; j++) {
                _sgrid[i][j] = _grid[i][j];
            }
        }
        _sgrid[_playerX][_playerY] = TileType::Player;

        _gold = 0;
        _steps = 0;
        _gameRunning = true;
        _gameWon = false;

        updateStatus();

        _pStatusBar->setFieldInfo(_width, _height);
        _pStatusBar->setMessage("Use WASD to move. Find the exit!");
    }

    void loadTileImages()
    {
        // FIX: Added "/images/" to match your MainView and your actual folder structure
        const char* imagePaths[] = {
            "res/images/empty.png",
            "res/images/start.png",
            "res/images/exit.png",
            "res/images/reward.png",
            "res/images/bandit.png",
            "res/images/mine.png",
            "res/images/wall.png",
            "res/images/player.png",
            "res/images/visited.png",
            "res/images/path.png"
        };

        for (int i = 0; i < 10; i++) {
            // FIX: Removed getResFileName because we are passing the full path now
            bool success = _tileImages[i].load(imagePaths[i]);

            // Helpful debug log if it fails
            if (!success) {
                td::String msg;
                msg.format("ERROR: Failed to load %s\n", imagePaths[i]);
                mu::dbgLog(msg.c_str());
            }
        }
    }

public:
    ViewDungeon(StatusBar* pStatusBar)
        : gui::Canvas({ gui::InputDevice::Event::Keyboard })
        , _pStatusBar(pStatusBar)
    {
        loadTileImages();
        initializeLevel();
    }

    void startGame()
    {
        initializeLevel();
        reDraw();
    }

    void setAlgorithm(Algorithm algo)
    {
        _currentAlgorithm = algo;
        _pStatusBar->setMessage("Algorithm changed");
    }

    void startVisualization()
    {
        _visualizationMode = true;
        _currentStep = 0;
        _pStatusBar->setMessage("Algorithm visualization started");
    }

    void stepVisualization()
    {
        if (_visualizationMode && _currentStep < _explorationPath.size()) {
            auto [x, y] = _explorationPath[_currentStep];
            _sgrid[x][y] = TileType::Visited;
            _currentStep++;
            reDraw();
        }
    }

    void resetGame()
    {
        startGame();
    }
};
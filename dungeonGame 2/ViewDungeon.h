#pragma once
#include <gui/Canvas.h>
#include <gui/Image.h>
#include <gui/Transformation.h>
#include <math/math.h>
#include <vector>
#include <time.h>
#include "StatusBar.h"

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
                _tileImages[static_cast<int>(_sgrid[i][j])].draw(_tileRect);
                tr.restoreContext();
            }
        }
    }

    void onKeyPressed(const gui::Key& key) override
    {
        if (!_gameRunning || _visualizationMode) return;

        td::INT4 newX = _playerX;
        td::INT4 newY = _playerY;

        switch (key.getCode())
        {
        case gui::Key::Code::Up:
        case gui::Key::Code::W:
            newY--;
            break;
        case gui::Key::Code::Down:
        case gui::Key::Code::S:
            newY++;
            break;
        case gui::Key::Code::Left:
        case gui::Key::Code::A:
            newX--;
            break;
        case gui::Key::Code::Right:
        case gui::Key::Code::D:
            newX++;
            break;
        default:
            return;
        }

        // Check bounds and walls
        if (newX < 0 || newX >= _width || newY < 0 || newY >= _height)
            return;
            
        if (_grid[newX][newY] == TileType::Wall)
            return;

        // Move player
        movePlayer(newX, newY);
        reDraw();
    }

    void movePlayer(td::INT4 newX, td::INT4 newY)
    {
        // Clear old player position
        _sgrid[_playerX][_playerY] = _grid[_playerX][_playerY];
        
        // Update position
        _playerX = newX;
        _playerY = newY;
        _steps++;
        
        // Handle tile effects
        handleTileEffect(newX, newY);
        
        // Update display
        _sgrid[_playerX][_playerY] = TileType::Player;
        
        // Update status
        updateStatus();
    }

    void handleTileEffect(td::INT4 x, td::INT4 y)
    {
        switch (_grid[x][y])
        {
        case TileType::Reward:
            _gold += 10;
            _pStatusBar->setMessage("Found reward! +10 gold");
            _grid[x][y] = TileType::Empty; // Collect reward
            break;
            
        case TileType::Bandit:
            _gold = _gold / 2; // Lose 50%
            _pStatusBar->setMessage("Bandit attacked! Lost 50% gold");
            _grid[x][y] = TileType::Empty; // Bandit leaves
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
        // Simple quiz implementation - can be enhanced with proper dialog
        bool correct = gui::Alert::showYesNoQuestion("Mine Quiz", 
            "What is 2+2?", "4", "5");
            
        if (correct) {
            _pStatusBar->setMessage("Quiz passed! Mine disarmed.");
            _grid[x][y] = TileType::Empty;
        } else {
            _pStatusBar->setMessage("Quiz failed! Lost 5 gold.");
            _gold = std::max(0, _gold - 5);
        }
    }

    void updateStatus()
    {
        td::String goldText;
        goldText.format("%d", _gold);
        _pStatusBar->setNoOfBombs(_gold); // Reusing bombs display for gold
        
        td::String stepsText;
        stepsText.format("%d", _steps);
        _pStatusBar->setNoOfFlags(_steps); // Reusing flags display for steps
    }

    void initializeLevel()
    {
        // Clear grids
        for (td::INT4 i = 0; i < _width; i++) {
            for (td::INT4 j = 0; j < _height; j++) {
                _grid[i][j] = TileType::Empty;
                _sgrid[i][j] = TileType::Empty;
            }
        }
        
        // Create a simple level layout
        _width = 12;
        _height = 10;
        
        // Borders
        for (td::INT4 i = 0; i < _width; i++) {
            _grid[i][0] = TileType::Wall;
            _grid[i][_height-1] = TileType::Wall;
        }
        for (td::INT4 j = 0; j < _height; j++) {
            _grid[0][j] = TileType::Wall;
            _grid[_width-1][j] = TileType::Wall;
        }
        
        // Start and exit
        _playerX = 1; _playerY = 1;
        _grid[1][1] = TileType::Start;
        _grid[_width-2][_height-2] = TileType::Exit;
        
        // Some obstacles and special tiles
        _grid[3][3] = TileType::Reward;
        _grid[5][5] = TileType::Bandit;
        _grid[7][7] = TileType::Mine;
        _grid[4][4] = TileType::Wall;
        _grid[4][5] = TileType::Wall;
        
        // Initialize display
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
        // Load images for each tile type - adjust paths as needed
        const char* imagePaths[] = {
            ":empty",      // Empty
            ":start",      // Start  
            ":exit",       // Exit
            ":reward",     // Reward
            ":bandit",     // Bandit
            ":mine",       // Mine
            ":wall",       // Wall
            ":player",     // Player
            ":visited",    // Visited
            ":path"        // Path
        };
        
        for (int i = 0; i < 10; i++) {
            _tileImages[i].load(gui::getResFileName(imagePaths[i]));
        }
    }

public:
    ViewDungeon(StatusBar* pStatusBar)
        : gui::Canvas({ gui::InputDevice::Event::KeyPressed })
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
        // TODO: Run selected algorithm and store path in _explorationPath
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
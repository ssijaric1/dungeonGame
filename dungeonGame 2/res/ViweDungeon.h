// ViewDungeon.h - COMPLETE CORRECTED VERSION
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
        Player
    };

    enum class Algorithm { DFS = 0, BFS, UCS, Greedy, AStar };
    enum class Difficulty { Easy = 0, Medium, Hard };

protected:
    gui::Image _tileImages[8]; // Only 8 tiles
    StatusBar* _pStatusBar;
    
    td::INT4 _width, _height;
    td::INT4 _playerX, _playerY;
    td::INT4 _gold;
    td::INT4 _steps;
    
    TileType _grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    TileType _sgrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    
    const gui::CoordType _tileSize = 32;
    gui::Rect _tileRect;
    
    bool _gameRunning = false;
    bool _gameWon = false;
    Algorithm _currentAlgorithm = Algorithm::DFS;
    Difficulty _difficulty = Difficulty::Medium;

protected:
    void onDraw(const gui::Rect& rect) override
    {
        for (td::INT4 i = 0; i < _width; i++) {
            for (td::INT4 j = 0; j < _height; j++) {
                _tileRect = { i * _tileSize, j * _tileSize, _tileSize };
                gui::Transformation::saveContext();
                gui::Transformation tr;
                _tileImages[static_cast<int>(_sgrid[i][j])].draw(_tileRect);
                tr.restoreContext();
            }
        }
    }

    void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override
    {
        if (!_gameRunning) return;
        
        gui::Point click = inputDevice.getFramePoint();
        td::INT4 x = click.x / _tileSize;
        td::INT4 y = click.y / _tileSize;
        
        // Check bounds
        if (x < 0 || x >= _width || y < 0 || y >= _height) return;
        
        // Handle tile click for mines (quizzes)
        if (_grid[x][y] == TileType::Mine) {
            showMineQuiz(x, y);
        }
    }

    void movePlayer(td::INT4 newX, td::INT4 newY)
    {
        if (!_gameRunning) return;
        
        // Check bounds
        if (newX < 0 || newX >= _width || newY < 0 || newY >= _height) {
            _pStatusBar->setMessage("Can't move outside!");
            return;
        }
        
        // Check for walls
        if (_grid[newX][newY] == TileType::Wall) {
            _pStatusBar->setMessage("Can't move through walls!");
            return;
        }
        
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
        reDraw();
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
            _pStatusBar->setMessage("Mine triggered!");
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
        // Simple math quiz
        int a = rand() % 10 + 1;
        int b = rand() % 10 + 1;
        int correct = a + b;
        
        td::String question;
        question.format("Mine Quiz: %d + %d = ?", a, b);
        
        td::String correctStr;
        correctStr.format("%d", correct);
        
        // Use simple Alert (not Yes/No for now)
        gui::Alert::show("Mine Quiz", question.c_str(), "OK", [this, x, y]() {
            _pStatusBar->setMessage("Quiz passed! Mine disarmed.");
            _grid[x][y] = TileType::Empty;
            _sgrid[x][y] = TileType::Empty;
            updateStatus();
            reDraw();
        });
    }

    void updateStatus()
    {
        _pStatusBar->setNoOfGold(_gold);
        _pStatusBar->setNoOfSteps(_steps);
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
        
        // Set difficulty-based parameters
        switch (_difficulty) {
            case Difficulty::Easy:
                _width = 12;
                _height = 10;
                break;
            case Difficulty::Medium:
                _width = 16;
                _height = 12;
                break;
            case Difficulty::Hard:
                _width = 20;
                _height = 15;
                break;
        }
        
        // Create borders
        for (td::INT4 i = 0; i < _width; i++) {
            _grid[i][0] = TileType::Wall;
            _grid[i][_height-1] = TileType::Wall;
            _sgrid[i][0] = TileType::Wall;
            _sgrid[i][_height-1] = TileType::Wall;
        }
        for (td::INT4 j = 0; j < _height; j++) {
            _grid[0][j] = TileType::Wall;
            _grid[_width-1][j] = TileType::Wall;
            _sgrid[0][j] = TileType::Wall;
            _sgrid[_width-1][j] = TileType::Wall;
        }
        
        // Start and exit positions
        _playerX = 1; 
        _playerY = 1;
        _grid[1][1] = TileType::Start;
        _sgrid[1][1] = TileType::Start;
        
        _grid[_width-2][_height-2] = TileType::Exit;
        _sgrid[_width-2][_height-2] = TileType::Exit;
        
        // Add some random obstacles
        for (int i = 0; i < (_width * _height) / 10; i++) {
            int x = rand() % (_width - 4) + 2;
            int y = rand() % (_height - 4) + 2;
            if (_grid[x][y] == TileType::Empty) {
                _grid[x][y] = TileType::Wall;
                _sgrid[x][y] = TileType::Wall;
            }
        }
        
        // Add rewards, bandits, and mines
        int numRewards = 3;
        int numBandits = 2;
        int numMines = 2;
        
        for (int i = 0; i < numRewards; i++) {
            placeRandomTile(TileType::Reward);
        }
        for (int i = 0; i < numBandits; i++) {
            placeRandomTile(TileType::Bandit);
        }
        for (int i = 0; i < numMines; i++) {
            placeRandomTile(TileType::Mine);
        }
        
        // Place player
        _grid[_playerX][_playerY] = TileType::Start;
        _sgrid[_playerX][_playerY] = TileType::Player;
        
        _gold = 0;
        _steps = 0;
        _gameRunning = true;
        _gameWon = false;
        
        updateStatus();
        _pStatusBar->setFieldInfo(_width, _height);
        _pStatusBar->setMessage("Use buttons to move. Find the exit!");
    }
    
    void placeRandomTile(TileType type)
    {
        int attempts = 0;
        while (attempts < 100) {
            int x = rand() % (_width - 4) + 2;
            int y = rand() % (_height - 4) + 2;
            
            if (_grid[x][y] == TileType::Empty) {
                _grid[x][y] = type;
                _sgrid[x][y] = type;
                return;
            }
            attempts++;
        }
    }

    void loadTileImages()
    {
        // Try to load images
        const char* imagePaths[] = {
            ":empty",      // Empty
            ":start",      // Start  
            ":exit",       // Exit
            ":reward",     // Reward
            ":bandit",     // Bandit
            ":mine",       // Mine
            ":wall",       // Wall
            ":player",     // Player
        };
        
        for (int i = 0; i < 8; i++) {
            _tileImages[i].load(gui::getResFileName(imagePaths[i]));
        }
    }

public:
    ViewDungeon(StatusBar* pStatusBar)
        : gui::Canvas({ gui::InputDevice::Event::PrimaryClicks })
        , _pStatusBar(pStatusBar)
        , _width(12)
        , _height(10)
        , _playerX(1)
        , _playerY(1)
        , _gold(0)
        , _steps(0)
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
    
    void setDifficulty(Difficulty diff)
    {
        _difficulty = diff;
        initializeLevel();
        reDraw();
    }
    
    // Movement methods for UI buttons
    void moveUp() { movePlayer(_playerX, _playerY - 1); }
    void moveDown() { movePlayer(_playerX, _playerY + 1); }
    void moveLeft() { movePlayer(_playerX - 1, _playerY); }
    void moveRight() { movePlayer(_playerX + 1, _playerY); }

    void resetGame()
    {
        startGame();
    }
};
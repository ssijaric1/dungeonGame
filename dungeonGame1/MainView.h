#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/ComboBox.h>
#include <gui/Button.h>
#include <gui/VerticalLayout.h>
#include <gui/HorizontalLayout.h>
#include <gui/GridLayout.h>
#include <td/ColorID.h>
#include <cnt/SafeFullVector.h>

enum class TileType {
    EMPTY,
    START,
    EXIT,
    REWARD,
    BANDIT,
    MINE,
    PLAYER
};

class MainView : public gui::View
{
private:
    // Simple layouts with required size parameters
    gui::HorizontalLayout _mainLayout{2};
    gui::VerticalLayout _controlsLayout{10}; // Increased for new buttons
    gui::GridLayout _gameGrid;
    
    // Control elements
    gui::Label _titleLabel;
    gui::ComboBox _algorithmCombo;
    gui::Button _startButton;
    gui::Button _stepButton;
    gui::Button _resetButton;
    gui::Label _statusLabel;
    gui::Label _goldLabel;
    gui::Label _stepsLabel;
    
    // Direction buttons
    gui::Button _upButton;
    gui::Button _downButton;
    gui::Button _leftButton;
    gui::Button _rightButton;
    gui::HorizontalLayout _directionLayout{4}; // For 4 direction buttons
    
    // Grid storage
    cnt::SafeFullVector<gui::Label*> _gridLabels;
    cnt::SafeFullVector<TileType> _gridTiles;
    static const int GRID_SIZE = 8;
    
    // Game state
    int _playerX, _playerY;
    int _gold;
    int _steps;
    bool _gameRunning;
    
public:
    MainView()
        : _gameGrid(GRID_SIZE, GRID_SIZE)
        , _titleLabel("Dungeon Pathfinder")
        , _startButton("Start")
        , _stepButton("Step") 
        , _resetButton("Reset")
        , _statusLabel("Ready to explore! Use direction buttons to move.")
        , _goldLabel("Gold: 0")
        , _stepsLabel("Steps: 0")
        , _upButton("Up")
        , _downButton("Down")
        , _leftButton("Left")
        , _rightButton("Right")
        , _gridLabels(GRID_SIZE * GRID_SIZE)
        , _gridTiles(GRID_SIZE * GRID_SIZE)
    {
        initializeGame();
        setupUI();
        setupEventHandlers();
    }
    
    ~MainView()
    {
        for (auto* label : _gridLabels) {
            delete label;
        }
    }
    
private:
    void initializeGame()
    {
        // Initialize all tiles as empty
        for (int i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
            _gridTiles[i] = TileType::EMPTY;
        }
        
        // Set up a simple level
        _playerX = 1;
        _playerY = 1;
        _gridTiles[_playerY * GRID_SIZE + _playerX] = TileType::START;
        _gridTiles[(GRID_SIZE-2) * GRID_SIZE + (GRID_SIZE-2)] = TileType::EXIT;
        
        // Add some rewards, bandits, and mines
        _gridTiles[2 * GRID_SIZE + 3] = TileType::REWARD;
        _gridTiles[3 * GRID_SIZE + 5] = TileType::REWARD;
        _gridTiles[5 * GRID_SIZE + 2] = TileType::BANDIT;
        _gridTiles[4 * GRID_SIZE + 4] = TileType::MINE;
        
        _gold = 0;
        _steps = 0;
        _gameRunning = true;
        
        updateStatus();
    }
    
    void setupUI()
    {
        // Create grid cells with proper tile representation
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                int index = row * GRID_SIZE + col;
                gui::Label* cell = new gui::Label(getTileSymbol(_gridTiles[index]));
                cell->setBackgroundColor(getTileColor(_gridTiles[index]));
                cell->setSizeLimits(40, gui::Control::Limit::Fixed, 40, gui::Control::Limit::Fixed);
                
                _gridLabels[index] = cell;
                _gameGrid.insert(row, col, *cell);
            }
        }
        
        // Update player position display
        updatePlayerPosition();
        
        // Setup algorithm combo
        _algorithmCombo.addItem("DFS");
        _algorithmCombo.addItem("BFS"); 
        _algorithmCombo.addItem("UCS");
        _algorithmCombo.addItem("Greedy");
        _algorithmCombo.addItem("A*");
        _algorithmCombo.selectIndex(0);
        
        // Setup direction buttons in horizontal layout
        _directionLayout.append(_upButton);
        _directionLayout.append(_downButton);
        _directionLayout.append(_leftButton);
        _directionLayout.append(_rightButton);
        
        // Add controls
        _controlsLayout.append(_titleLabel);
        _controlsLayout.append(_algorithmCombo);
        _controlsLayout.append(_startButton);
        _controlsLayout.append(_stepButton);
        _controlsLayout.append(_resetButton);
        
        // Add direction label and buttons
        _controlsLayout.append(*(new gui::Label("Movement:")));
        _controlsLayout.append(_directionLayout);
        
        _controlsLayout.append(_goldLabel);
        _controlsLayout.append(_stepsLabel);
        _controlsLayout.append(_statusLabel);
        
        // Simple main layout
        _mainLayout.append(_gameGrid);
        _mainLayout.append(_controlsLayout);
        
        setLayout(&_mainLayout);
    }
    
    const char* getTileSymbol(TileType type)
    {
        switch(type) {
            case TileType::START: return "S";
            case TileType::EXIT: return "E";
            case TileType::REWARD: return "R";
            case TileType::BANDIT: return "B";
            case TileType::MINE: return "M";
            case TileType::PLAYER: return "P";
            case TileType::EMPTY: 
            default: return " ";
        }
    }
    
    td::ColorID getTileColor(TileType type)
    {
        switch(type) {
            case TileType::START: return td::ColorID::Green;
            case TileType::EXIT: return td::ColorID::Red;
            case TileType::REWARD: return td::ColorID::Gold;
            case TileType::BANDIT: return td::ColorID::DarkRed;
            case TileType::MINE: return td::ColorID::Orange;
            case TileType::PLAYER: return td::ColorID::Blue;
            case TileType::EMPTY: 
            default: 
                return td::ColorID::LightGray;
        }
    }
    
    void updatePlayerPosition()
    {
        // Clear previous player position from all tiles
        for (int i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
            if (_gridLabels[i] != nullptr) {
                int row = i / GRID_SIZE;
                int col = i % GRID_SIZE;
                
                // Reset tile to its original type (except where player is)
                if (!(row == _playerY && col == _playerX)) {
                    _gridLabels[i]->setTitle(getTileSymbol(_gridTiles[i]));
                    _gridLabels[i]->setBackgroundColor(getTileColor(_gridTiles[i]));
                }
            }
        }
        
        // Set player position
        int playerIndex = _playerY * GRID_SIZE + _playerX;
        if (_gridLabels[playerIndex] != nullptr) {
            _gridLabels[playerIndex]->setTitle("P");
            _gridLabels[playerIndex]->setBackgroundColor(td::ColorID::Blue);
        }
    }
    
    void updateStatus()
    {
        // Update gold display
        char goldText[20];
        snprintf(goldText, sizeof(goldText), "Gold: %d", _gold);
        _goldLabel.setTitle(goldText);
        
        // Update steps display
        char stepsText[20];
        snprintf(stepsText, sizeof(stepsText), "Steps: %d", _steps);
        _stepsLabel.setTitle(stepsText);
    }
    
    void movePlayer(int newX, int newY)
    {
        if (!_gameRunning) return;
        
        // Check bounds
        if (newX < 0 || newX >= GRID_SIZE || newY < 0 || newY >= GRID_SIZE)
            return;
        
        // Update position
        _playerX = newX;
        _playerY = newY;
        _steps++;
        
        // Handle tile effects
        int newIndex = _playerY * GRID_SIZE + _playerX;
        TileType newTile = _gridTiles[newIndex];
        
        switch(newTile) {
            case TileType::REWARD:
                _gold += 10;
                _statusLabel.setTitle("Found reward! +10 gold");
                _gridTiles[newIndex] = TileType::EMPTY; // Reward collected
                break;
                
            case TileType::BANDIT:
                _gold = _gold / 2; // Lose 50%
                _statusLabel.setTitle("Bandit attacked! Lost 50% gold");
                _gridTiles[newIndex] = TileType::EMPTY; // Bandit gone
                break;
                
            case TileType::MINE:
                _statusLabel.setTitle("Mine triggered! Quiz coming soon...");
                // TODO: Implement quiz popup
                break;
                
            case TileType::EXIT:
                _gameRunning = false;
                _statusLabel.setTitle("You reached the exit! You win!");
                break;
                
            default:
                break;
        }
        
        updatePlayerPosition();
        updateStatus();
    }
    
    void setupEventHandlers()
    {
        _startButton.onClick([this]() {
            _statusLabel.setTitle("Game Started!");
            initializeGame();
            updatePlayerPosition();
        });
        
        _stepButton.onClick([this]() {
            // For algorithm step-through
            _statusLabel.setTitle("Algorithm step executed");
        });
        
        _resetButton.onClick([this]() {
            initializeGame();
            updatePlayerPosition();
            _statusLabel.setTitle("Game Reset");
        });
        
        // Direction button handlers
        _upButton.onClick([this]() {
            movePlayer(_playerX, _playerY - 1);
        });
        
        _downButton.onClick([this]() {
            movePlayer(_playerX, _playerY + 1);
        });
        
        _leftButton.onClick([this]() {
            movePlayer(_playerX - 1, _playerY);
        });
        
        _rightButton.onClick([this]() {
            movePlayer(_playerX + 1, _playerY);
        });
    }
};
#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/Image.h>
#include <gui/Canvas.h>
#include <gui/ComboBox.h>
#include <gui/Button.h>
#include <gui/VerticalLayout.h>
#include <gui/HorizontalLayout.h>
#include <gui/GridLayout.h>
#include <td/ColorID.h>
#include <cnt/SafeFullVector.h>
#include <gui/Transformation.h>

enum class TileType {
    EMPTY = 0,
    START = 1,
    EXIT = 2,
    REWARD = 3,
    BANDIT = 4,
    MINE = 5,
    PLAYER = 6,
    WALL= 7,
    VISITED=8,
    PATH=9
};

// Tile widget that displays an image using Canvas
class TileWidget : public gui::Canvas
{
private:
    gui::Image _image;
    TileType _type;
    bool _imageLoaded;

public:
    TileWidget()
        : gui::Canvas({})
        , _type(TileType::EMPTY)
        , _imageLoaded(false)
    {
        //setSizeLimits(40, gui::Control::Limit::Fixed, 40, gui::Control::Limit::Fixed);
        setSizeLimits(95, gui::Control::Limit::UseAsMin, 95, gui::Control::Limit::UseAsMin);
    }

    void loadImage(const char* imagePath,int i)
    {   
        if (_imageLoaded){
            _image = gui::Image(imagePath);
        }
        else {
        _imageLoaded = _image.load(imagePath);

        }
        


        if (!_imageLoaded) {
            td::String msg;
            msg.format("TileWidget: Failed to load '%s'\n", imagePath);
            mu::dbgLog(msg.c_str());
        }

        reDraw();
    }

    void setType(TileType type)
    {
        _type = type;
        reDraw();
    }

protected:
    void onDraw(const gui::Rect& rect) override
    {
        //gui::Rect drawRect = { 0, 0, 40, 40 };
        gui::Rect drawRect = rect;
        drawRect.isNormalized();


        if (_imageLoaded) {
            // Draw the image
            gui::Transformation::saveContext();
            gui::Transformation tr;
            _image.draw(drawRect);
            tr.restoreContext();
        }
        else {
            // Fallback: colored background with letter
            td::ColorID color = td::ColorID::LightGray;
            const char* symbol = " ";

            switch (_type) {
            case TileType::START:
                color = td::ColorID::Green;
                symbol = "S";
                break;
            case TileType::EXIT:
                color = td::ColorID::Red;
                symbol = "E";
                break;
            case TileType::REWARD:
                color = td::ColorID::Gold;
                symbol = "R";
                break;
            case TileType::BANDIT:
                color = td::ColorID::DarkRed;
                symbol = "B";
                break;
            case TileType::MINE:
                color = td::ColorID::Orange;
                symbol = "M";
                break;
            case TileType::PLAYER:
                color = td::ColorID::Blue;
                symbol = "P";
                break;
            default:
                color = td::ColorID::LightGray;
                symbol = " ";
                break;
            }

            setBackgroundColor(td::ColorID::BurlyWood);
        }
    }
};

class MainView : public gui::View
{
private:
    gui::HorizontalLayout _mainLayout{ 2 };
    gui::VerticalLayout _controlsLayout{ 10 };
    gui::GridLayout _gameGrid;

    class KeyboardCanvas : public gui::Canvas
    {
    private:
        MainView* _pMainView;

    public:
        KeyboardCanvas(MainView* pMainView)
            : gui::Canvas({ gui::InputDevice::Event::Keyboard })
            , _pMainView(pMainView)
        {
            setSizeLimits(0, gui::Control::Limit::Fixed, 0, gui::Control::Limit::Fixed);
        }

    protected:
        bool onKeyPressed(const gui::Key& key) override
        {
            // Forward to MainView
            return _pMainView->handleKeyPressed(key);
        }
    };

    KeyboardCanvas _keyboardCanvas;

    gui::Label _titleLabel;
    gui::ComboBox _algorithmCombo;
    gui::Button _startButton;
    gui::Button _stepButton;
    gui::Button _resetButton;
    gui::Label _statusLabel;
    gui::Label _goldLabel;
    gui::Label _stepsLabel;

    gui::Button _upButton;
    gui::Button _downButton;
    gui::Button _leftButton;
    gui::Button _rightButton;
    gui::HorizontalLayout _directionLayout{ 4 };

    cnt::SafeFullVector<TileWidget*> _gridTiles;
    cnt::SafeFullVector<TileType> _tileTypes;
    static const int GRID_SIZE = 8;

    // Store loaded images
    gui::Image _tileImages[10];
    bool _imagesLoaded;

    // Image resource IDs (must match IDs in DevRes.xml)
    const char* _imagePaths[10] = {
        ":empty",      // EMPTY
        ":start_tile", // START (renamed to avoid conflict with arrow icon)
        ":exit",       // EXIT
        ":reward",     // REWARD
        ":bandit",     // BANDIT
        ":mine",       // MINE
        ":player",// PLAYER
        ":wall",
        ":visited",
        ":path"
    };

    int _playerX, _playerY;
    int _gold;
    int _steps;
    bool _gameRunning;

public:
    MainView()
        //: gui::View({ gui::InputDevice::Event::Keyboard })
        : _gameGrid(GRID_SIZE, GRID_SIZE)
        , _keyboardCanvas(this)
        , _titleLabel("Dungeon Pathfinder")
        , _startButton("Start")
        , _stepButton("Step")
        , _resetButton("Reset")
        , _statusLabel("Ready to explore! Use buttons to move.")
        , _goldLabel("Gold: 0")
        , _stepsLabel("Steps: 0")
        , _upButton("↑ Up")
        , _downButton(" Down")
        , _leftButton("← Left")
        , _rightButton("→ Right")
        , _gridTiles(GRID_SIZE* GRID_SIZE)
        , _tileTypes(GRID_SIZE* GRID_SIZE)
        , _imagesLoaded(true)
    {
        //loadTileImages();
        initializeGame();
        setupUI();
        setupEventHandlers();
    }

    ~MainView()
    {
        for (auto* tile : _gridTiles) {
            delete tile;
        }
    }

    bool handleKeyPressed(const gui::Key& key)
    {
        if (!_gameRunning)
            return false;

        int newX = _playerX;
        int newY = _playerY;

        // Check if it's a virtual key (arrow keys)
        if (key.isVirtual())
        {
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
        // Check for WASD (ASCII keys)
        else if (key.isASCII())
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

        // Move player using your existing method
        movePlayer(newX, newY);

        return true; // Key handled successfully
    }
private:


    void initializeGame()
    {
        // Initialize all tiles as empty
        for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
            _tileTypes[i] = TileType::EMPTY;
        }

        // Set up level
        _playerX = 1;
        _playerY = 1;
        _tileTypes[_playerY * GRID_SIZE + _playerX] = TileType::START;
        _tileTypes[(GRID_SIZE - 2) * GRID_SIZE + (GRID_SIZE - 2)] = TileType::EXIT;

        // Add rewards, bandits, mines
        _tileTypes[2 * GRID_SIZE + 3] = TileType::REWARD;
        _tileTypes[3 * GRID_SIZE + 5] = TileType::REWARD;
        _tileTypes[5 * GRID_SIZE + 2] = TileType::BANDIT;
        _tileTypes[4 * GRID_SIZE + 4] = TileType::MINE;

        _gold = 0;
        _steps = 0;
        _gameRunning = true;

        updateStatus();
    }

    void setupUI()
    {
        _gameGrid.setSpaceBetweenCells(5, 0);
        _gameGrid.setMargins(0, 0);
        // Create grid cells
        for (int row = 0; row < GRID_SIZE; row++) {
            for (int col = 0; col < GRID_SIZE; col++) {
                auto k = col;
                auto r = row;
                int index = row * GRID_SIZE + col;
                TileWidget* cell = new TileWidget();

                TileType tileType = _tileTypes[index];
                int typeIndex = static_cast<int>(tileType);
                if (index > 63) {
                    auto k = index;
                };

                // Load the appropriate image
                if (_imagesLoaded) {
                    cell->loadImage(_imagePaths[typeIndex],index);
                }
                else {
                    cell->setType(tileType);
                }

                _gridTiles[index] = cell;
                //_gameGrid.insert(row, col, *cell);
                _gameGrid.insert(row, col, *cell, td::HAlignment::Center, td::VAlignment::Center);
            }
        }
        
        // Show player
        updatePlayerPosition();

        // Setup algorithm combo
        _algorithmCombo.addItem("DFS");
        _algorithmCombo.addItem("BFS");
        _algorithmCombo.addItem("UCS");
        _algorithmCombo.addItem("Greedy");
        _algorithmCombo.addItem("A*");
        _algorithmCombo.selectIndex(0);

        // Direction buttons
        _directionLayout.append(_upButton);
        _directionLayout.append(_downButton);
        _directionLayout.append(_leftButton);
        _directionLayout.append(_rightButton);

        // Add all controls
        _controlsLayout.append(_titleLabel);
        _controlsLayout.append(_algorithmCombo);
        _controlsLayout.append(_startButton);
        _controlsLayout.append(_stepButton);
        _controlsLayout.append(_resetButton);
        _controlsLayout.append(*(new gui::Label("Movement:")));
        _controlsLayout.append(_directionLayout);
        _controlsLayout.append(_goldLabel);
        _controlsLayout.append(_stepsLabel);
        _controlsLayout.append(_statusLabel);

        _mainLayout.append(_gameGrid);
        _mainLayout.append(_controlsLayout);
        setLayout(&_mainLayout);
     
       // _mainLayout.append(_gameGrid);
       // _mainLayout.append(_controlsLayout);
        //_mainLayout.setLayout(&_mainLayout);
    }

    void updatePlayerPosition()
    {
        // Update all tiles
        for (int i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
            if (_gridTiles[i] != nullptr) {
                int row = i / GRID_SIZE;
                int col = i % GRID_SIZE;

                if (row == _playerY && col == _playerX) {
                    if (_imagesLoaded) {
                        _gridTiles[i]->loadImage(_imagePaths[static_cast<int>(TileType::PLAYER)],i);
                    }
                    else {
                        _gridTiles[i]->setType(TileType::PLAYER);
                    }
                }
                else {
                    TileType tileType = _tileTypes[i];
                    if (_imagesLoaded) {
                        auto k = static_cast<int>(tileType);
                        _gridTiles[i]->loadImage(_imagePaths[static_cast<int>(tileType)], i);
                    }
                    else {
                        _gridTiles[i]->setType(tileType);
                    }
                }
            }
        }
    }

    void updateStatus()
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Gold: %d", _gold);
        _goldLabel.setTitle(buffer);

        snprintf(buffer, sizeof(buffer), "Steps: %d", _steps);
        _stepsLabel.setTitle(buffer);
    }

    void movePlayer(int newX, int newY)
    {
        if (!_gameRunning) return;

        // Check bounds
        if (newX < 0 || newX >= GRID_SIZE || newY < 0 || newY >= GRID_SIZE) {
            _statusLabel.setTitle("Can't move there - out of bounds!");
            return;
        }

        // Update position
        _playerX = newX;
        _playerY = newY;
        _steps++;

        // Handle tile effects
        int newIndex = _playerY * GRID_SIZE + _playerX;
        TileType newTile = _tileTypes[newIndex];

        switch (newTile) {
        case TileType::REWARD:
            _gold += 10;
            _statusLabel.setTitle("Found reward! +10 gold");
            _tileTypes[newIndex] = TileType::EMPTY;
            break;

        case TileType::BANDIT:
            _gold = _gold / 2;
            _statusLabel.setTitle("Bandit attacked! Lost 50% gold");
            _tileTypes[newIndex] = TileType::EMPTY;
            break;

        case TileType::MINE:
            _statusLabel.setTitle("Mine triggered! Quiz coming soon...");
            // TODO: Implement quiz
            break;

        case TileType::EXIT:
            _gameRunning = false;
            _statusLabel.setTitle("*** YOU WIN! You reached the exit! ***");
            break;

        default:
            _statusLabel.setTitle("Moving...");
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
            _statusLabel.setTitle("Algorithm step - not yet implemented");
            });

        _resetButton.onClick([this]() {
            initializeGame();
            updatePlayerPosition();
            _statusLabel.setTitle("Game Reset! Ready to play.");
            });

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
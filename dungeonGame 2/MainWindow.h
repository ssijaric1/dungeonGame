// MainWindow.h - SIMPLE WORKING VERSION
#pragma once
#include <gui/Window.h>
#include <gui/Button.h>
#include <gui/VerticalLayout.h>
#include <gui/HorizontalLayout.h>
#include "StatusBar.h"
#include "ViewDungeon.h"

class MainWindow : public gui::Window
{
protected:
    StatusBar _statusBar;
    ViewDungeon _mainView;
    
    // Control buttons
    gui::Button _startButton;
    gui::Button _resetButton;
    gui::Button _stepButton;
    
    // Movement buttons
    gui::Button _upButton;
    gui::Button _downButton;
    gui::Button _leftButton;
    gui::Button _rightButton;
    
public:
    MainWindow()
        : gui::Window(gui::Size(800, 600))
        , _statusBar()
        , _mainView(&_statusBar)
        , _startButton("Start Game")
        , _resetButton("Reset")
        , _stepButton("Step")
        , _upButton("Up")
        , _downButton("Down")
        , _leftButton("Left")
        , _rightButton("Right")
    {
        setTitle("Dungeon Pathfinder");
        setupUI();
        setStatusBar(_statusBar);
    }
    
private:
    void setupUI()
    {
        // Create a vertical layout for everything
        gui::VerticalLayout* mainLayout = new gui::VerticalLayout(2);
        
        // Create a horizontal layout for control buttons
        gui::HorizontalLayout* controlLayout = new gui::HorizontalLayout(4);
        controlLayout->append(_startButton);
        controlLayout->append(_resetButton);
        controlLayout->append(_stepButton);
        
        // Create a horizontal layout for movement buttons
        gui::HorizontalLayout* movementLayout = new gui::HorizontalLayout(4);
        movementLayout->append(_upButton);
        movementLayout->append(_downButton);
        movementLayout->append(_leftButton);
        movementLayout->append(_rightButton);
        
        // Add everything to main layout
        mainLayout->append(*controlLayout);
        mainLayout->append(*movementLayout);
        mainLayout->append(_mainView); // Takes remaining space
        
        // Create a view to hold everything
        gui::View* contentView = new gui::View();
        contentView->setLayout(mainLayout);
        
        setCentralView(contentView);
        
        // Setup event handlers
        setupEventHandlers();
    }
    
    void setupEventHandlers()
    {
        _startButton.onClick([this]() {
            _mainView.startGame();
        });
        
        _resetButton.onClick([this]() {
            _mainView.resetGame();
        });
        
        _stepButton.onClick([this]() {
            // Simple step action
            _statusBar.setMessage("Step button clicked");
        });
        
        _upButton.onClick([this]() {
            _mainView.moveUp();
        });
        
        _downButton.onClick([this]() {
            _mainView.moveDown();
        });
        
        _leftButton.onClick([this]() {
            _mainView.moveLeft();
        });
        
        _rightButton.onClick([this]() {
            _mainView.moveRight();
        });
    }
};
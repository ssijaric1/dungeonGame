#pragma once
#include <gui/Window.h>
#include "MainView.h"

class MainWindow : public gui::Window
{
protected:
    MainView _mainView;
    
public:
    MainWindow()
        : gui::Window(gui::Size(800, 600))
    {
        setTitle("Dungeon Pathfinder");
        setCentralView(&_mainView);
    }
    
    void onInitialAppearance() override
    {
        _mainView.setFocus();
    }
};
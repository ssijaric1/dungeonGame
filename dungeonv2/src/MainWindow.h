#pragma once
#include <gui/Window.h>
#include "MainView.h"

class MainWindow : public gui::Window {
protected:
    MainView view;

public:
    MainWindow() : gui::Window(gui::Size(1200, 800)) {  // Changed from Geometry to Size
        this->setTitle("Port Logistics Simulation");
        this->setCentralView(&view);
    }
};
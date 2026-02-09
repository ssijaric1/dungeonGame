#pragma once
#include <gui/View.h>
#include <gui/GridLayout.h>
#include <gui/HorizontalLayout.h>
#include "SimulationCanvas.h"

class MainView : public gui::View {
private:
    SimulationCanvas canvas;
    gui::HorizontalLayout hLayout;
    gui::GridLayout gridLayout;
    
public:
    MainView();
    virtual ~MainView() = default;
};
#pragma once
#include <gui/View.h>
#include <gui/Button.h>
#include <gui/GridLayout.h>
#include <gui/HorizontalLayout.h>
#include <gui/VerticalLayout.h>
#include "SimulationCanvas.h"

class MainView : public gui::View {
protected:
    SimulationCanvas canvas;
    
    // Layouts
    gui::HorizontalLayout hLayout;  // Main horizontal layout
    gui::GridLayout gridLayout;     // Grid layout: 2 rows, 1 column
    
public:
    MainView();
    
    // Add virtual destructor
    virtual ~MainView() = default;
};
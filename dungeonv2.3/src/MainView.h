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
    
    // Movement buttons
    gui::Button btnUp;
    gui::Button btnDown;
    gui::Button btnLeft;
    gui::Button btnRight;
    
    // Layouts
    gui::HorizontalLayout hLayout;  // Main horizontal layout
    gui::GridLayout gridLayout;     // Grid layout: 2 rows, 1 column
    
public:
    MainView();
    
    // Event handler for button clicks
    bool onClick(gui::Button* pBtn) override;
    
    // Make sure we include the base class virtual functions
    using gui::View::onClick;  // For other onClick overloads if they exist
    
    // Add virtual destructor
    virtual ~MainView() = default;
};
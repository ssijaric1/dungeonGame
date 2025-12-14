#include "MainView.h"

MainView::MainView() 
    : btnUp("↑ Up"),
      btnDown("↓ Down"),
      btnLeft("← Left"),
      btnRight("→ Right"),
      hLayout(10),        // 10 pixel spacing
      gridLayout(2, 2)    // 2 rows, 1 column
{
    // Create two horizontal layouts for the two rows
    gui::HorizontalLayout* row1Layout = new gui::HorizontalLayout(5);  // Row 1: Up/Down
    gui::HorizontalLayout* row2Layout = new gui::HorizontalLayout(5);  // Row 2: Left/Right
    
    // Setup first row: spacer, up, down, spacer
    row1Layout->append(btnUp);    // Up button
    row1Layout->append(btnDown);  // Down button
    
    // Setup second row: spacer, left, right, spacer
    row2Layout->append(btnLeft);  // Left button
    row2Layout->append(btnRight); // Right button
    
    // Insert the horizontal layouts into the grid
    gridLayout.insert(0, 0, *row1Layout);  // Row 0: first horizontal layout
    gridLayout.insert(1, 0, *row2Layout);  // Row 1: second horizontal layout
    
    // Add canvas and grid layout to main horizontal layout
    hLayout << canvas << gridLayout;
    
    // Set this layout as the view's layout
    setLayout(&hLayout);
}

bool MainView::onClick(gui::Button* pBtn) {
    if (pBtn == &btnUp) {
        canvas.moveUp();
        return true;
    }
    else if (pBtn == &btnDown) {
        canvas.moveDown();
        return true;
    }
    else if (pBtn == &btnLeft) {
        canvas.moveLeft();
        return true;
    }
    else if (pBtn == &btnRight) {
        canvas.moveRight();
        return true;
    }
    
    return false;
}

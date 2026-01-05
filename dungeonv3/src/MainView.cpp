#include "MainView.h"

MainView::MainView() 
    : btnUp("↑ Up"),
      btnDown("↓ Down"),
      btnLeft("← Left"),
      btnRight("→ Right"),
      hLayout(1),        // 10 pixel spacing
      gridLayout(5, 6)    // 2 rows, 1 column
{
    gridLayout.insert(0, 3, btnUp);
    gridLayout.insert(2, 1, btnLeft);
    gridLayout.insert(2, 5, btnRight);
    gridLayout.insert(4, 3, btnDown);
    gridLayout.insert(2, 0, canvas);

   
    
    
    // Set this layout as the view's layout
    setLayout(&gridLayout);
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

#include "MainView.h"

MainView::MainView() 
    : hLayout(1)
    , gridLayout(1, 1) {
    gridLayout.insert(0, 0, canvas);
    setLayout(&gridLayout);
    canvas.setFocus(true);
}
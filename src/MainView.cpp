#include "MainView.h"
#include "DialogQuiz.h"

MainView::MainView()
    : canvas()
    , _lblAlgorithm(tr("Select Algorithm:"))
    , _cmbAlgorithm()
    , _lblCurrentGold(tr("Current Gold"))
    , _lblCurrentGoldValue("0")
    , _lblStatus(tr("Status"))
    , _lblStatusValue(tr("Ready"))
    , _lblPathLength(tr("Path Length"))
    , _lblPathLengthValue("0")
    , _lblNodesExplored(tr("Nodes Explored"))
    , _lblNodesExploredValue("0")
    , _btnStart(tr("START"))
    , _btnPause(tr("PAUSE"))
    , _btnStep(tr("STEP"))
    , _btnReset(tr("RESET"))
    , _btnGenerateNew(tr("GENERATE NEW DUNGEON"))
    , _lblAlgorithmComparison(tr("Algorithm Comparison"))
    , _lblComparisonPlaceholder(tr("Run algorithms to see metrics"))
    , _statsLayout(4, 2)
    , _controlPanel(20)
    , _mainLayout(1, 2)
{
    // THIS IS THE KEY LINE - connect canvas to this view
    canvas.setParent(this);

    _cmbAlgorithm.addItem(tr("A* Search"));
    _cmbAlgorithm.addItem(tr("Dijkstra"));
    _cmbAlgorithm.addItem(tr("Greedy Best-First"));
    _cmbAlgorithm.addItem(tr("Breadth-First Search"));
    _cmbAlgorithm.selectIndex(0);

    _lblCurrentGoldValue.setFont(gui::Font::ID::SystemLargestBold);
    _lblStatusValue.setFont(gui::Font::ID::SystemLargestBold);
    _lblPathLengthValue.setFont(gui::Font::ID::SystemBold);
    _lblNodesExploredValue.setFont(gui::Font::ID::SystemBold);

    gui::GridComposer gcStats(_statsLayout);
    gcStats.appendRow(_lblCurrentGold) << _lblStatus;
    gcStats.appendRow(_lblCurrentGoldValue) << _lblStatusValue;
    gcStats.appendRow(_lblPathLength) << _lblNodesExplored;
    gcStats.appendRow(_lblPathLengthValue) << _lblNodesExploredValue;

    _controlPanel.append(_lblAlgorithm);
    _controlPanel.append(_cmbAlgorithm);
    _controlPanel.appendSpace(20);
    _controlPanel.appendLayout(_statsLayout);
    _controlPanel.appendSpace(20);
    _controlPanel.append(_btnStart);
    _controlPanel.appendSpace(5);
    _controlPanel.append(_btnPause);
    _controlPanel.appendSpace(5);
    _controlPanel.append(_btnStep);
    _controlPanel.appendSpace(5);
    _controlPanel.append(_btnReset);
    _controlPanel.appendSpace(20);
    _controlPanel.append(_btnGenerateNew);
    _controlPanel.appendSpace(20);
    _controlPanel.append(_lblAlgorithmComparison);
    _controlPanel.appendSpace(5);
    _controlPanel.append(_lblComparisonPlaceholder);
    _controlPanel.appendSpacer();

    gui::GridComposer gc(_mainLayout);
    gc.appendRow(canvas) << _controlPanel;

    setLayout(&_mainLayout);
}

void MainView::updateStats() {
    // Update current gold - uses canvas.getGold() which returns gameState.gold
    td::String goldStr;
    goldStr.format("%d", canvas.getGold());
    _lblCurrentGoldValue.setTitle(goldStr);

    // Update status
    if (canvas.isGameOver()) {
        if (canvas.isGameWon()) {
            _lblStatusValue.setTitle(tr("Reached the Exit!"));  // Changed from "Won!"
        }
        else {
            _lblStatusValue.setTitle(tr("Game Over"));
        }
    }
    else {
        _lblStatusValue.setTitle(tr("Ready"));
    }

    // TODO: Update path length and nodes explored when algorithms are implemented
}

void MainView::showQuizDialog() {
    // Get parent frame through the view hierarchy
    auto* pFrame = getParentFrame();
    if (pFrame) {
        DialogQuiz* pDlg = new DialogQuiz(pFrame, 100); // 100 is dialog ID
        pDlg->open();
    }
}

bool MainView::onFinishDialog(gui::Dialog* pDlg) {
    // Cast to DialogQuiz to access quiz-specific methods
    DialogQuiz* pQuizDlg = dynamic_cast<DialogQuiz*>(pDlg);

    if (pQuizDlg) {
        // Check if user submitted answer (clicked OK)
        if (pDlg->getClickedButtonID() == gui::Dialog::Button::ID::OK) {
            bool answeredCorrectly = pQuizDlg->wasAnsweredCorrectly();
            canvas.handleQuizResult(answeredCorrectly);
        }
        else {
            // User cancelled - treat as wrong answer
            canvas.handleQuizResult(false);
        }

        return true;
    }
    return false;
}

bool MainView::onClick(gui::Button* pBtn) {
    if (pBtn == &_btnStart) {
        _lblStatusValue.setTitle(tr("Running"));
        canvas.setFocus();
        return true;
    }
    else if (pBtn == &_btnPause) {
        _lblStatusValue.setTitle(tr("Paused"));
        canvas.setFocus();
        return true;
    }
    else if (pBtn == &_btnStep) {
        _lblStatusValue.setTitle(tr("Stepping"));
        canvas.setFocus();
        return true;
    }
    else if (pBtn == &_btnReset) {
        canvas.resetGame();
        updateStats();
        _lblStatusValue.setTitle(tr("Ready"));
        _lblPathLengthValue.setTitle("0");
        _lblNodesExploredValue.setTitle("0");
        canvas.setFocus();
        return true;
    }
    else if (pBtn == &_btnGenerateNew) {
        canvas.resetGame();
        updateStats();
        _lblStatusValue.setTitle(tr("Ready"));
        _lblPathLengthValue.setTitle("0");
        _lblNodesExploredValue.setTitle("0");
        canvas.setFocus();
        return true;
    }

    return false;
}

// ====================================================================
// SimulationCanvas method implementations
// These are here because they call methods on MainView* which needs
// to be a complete type
// ====================================================================

void SimulationCanvas::showQuiz() {
    _waitingForQuizResult = true;
    if (_pParent) {
        _pParent->showQuizDialog();
    }
}

void SimulationCanvas::handleQuizResult(bool answeredCorrectly) {
    if (_waitingForQuizResult) {
        _waitingForQuizResult = false;
        gameState.resolveMineTile(answeredCorrectly);

        if (!answeredCorrectly) {
            showAlert(tr("Game Over"), tr("Wrong answer! You stepped on a mine."));
        }

        reDraw();
        if (_pParent) {
            _pParent->updateStats();
        }
    }
}

void SimulationCanvas::moveUp() {
    if (!gameState.gameOver) {
        bool hitMine = false;
        if (gameState.movePlayer(gameState.playerX, gameState.playerY - 1, hitMine)) {
            if (hitMine) {
                showQuiz();
            }
            reDraw();
            if (_pParent) {
                _pParent->updateStats();
            }
        }
    }
}

void SimulationCanvas::moveDown() {
    if (!gameState.gameOver) {
        bool hitMine = false;
        if (gameState.movePlayer(gameState.playerX, gameState.playerY + 1, hitMine)) {
            if (hitMine) {
                showQuiz();
            }
            reDraw();
            if (_pParent) {
                _pParent->updateStats();
            }
        }
    }
}

void SimulationCanvas::moveLeft() {
    if (!gameState.gameOver) {
        bool hitMine = false;
        if (gameState.movePlayer(gameState.playerX - 1, gameState.playerY, hitMine)) {
            if (hitMine) {
                showQuiz();
            }
            reDraw();
            if (_pParent) {
                _pParent->updateStats();
            }
        }
    }
}

void SimulationCanvas::moveRight() {
    if (!gameState.gameOver) {
        bool hitMine = false;
        if (gameState.movePlayer(gameState.playerX + 1, gameState.playerY, hitMine)) {
            if (hitMine) {
                showQuiz();
            }
            reDraw();
            if (_pParent) {
                _pParent->updateStats();
            }
        }
    }
}
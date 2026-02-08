#pragma once
#include <gui/View.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/Label.h>
#include <gui/Button.h>
#include <gui/ComboBox.h>
#include <gui/VerticalLayout.h>
#include "SimulationCanvas.h"

class DialogQuiz; // Forward declaration

class MainView : public gui::View {
protected:
    SimulationCanvas canvas;

    // Right panel controls
    gui::Label _lblAlgorithm;
    gui::ComboBox _cmbAlgorithm;

    gui::Label _lblCurrentGold;
    gui::Label _lblCurrentGoldValue;
    gui::Label _lblStatus;
    gui::Label _lblStatusValue;

    gui::Label _lblPathLength;
    gui::Label _lblPathLengthValue;
    gui::Label _lblNodesExplored;
    gui::Label _lblNodesExploredValue;

    gui::Button _btnStart;
    gui::Button _btnPause;
    gui::Button _btnStep;
    gui::Button _btnReset;
    gui::Button _btnGenerateNew;

    gui::Label _lblAlgorithmComparison;
    gui::Label _lblComparisonPlaceholder;

    // Layouts
    gui::GridLayout _statsLayout;
    gui::VerticalLayout _controlPanel;
    gui::GridLayout _mainLayout;

public:
    MainView();

    void focusOnCanvas() {
        canvas.setFocus();
    }

    bool onClick(gui::Button* pBtn) override;

    // Add method to update stats display
    void updateStats();

    // Methods for quiz dialog
    void showQuizDialog();
    bool onFinishDialog(gui::Dialog* pDlg);

    virtual ~MainView() = default;
};
//
//  DialogQuiz.h
//  Quiz dialog for mine tiles
//
#pragma once
#include <gui/Dialog.h>
#include "ViewQuiz.h"

class DialogQuiz : public gui::Dialog
{
protected:
    ViewQuiz _viewQuiz;
    bool _answeredCorrectly = false;
    bool _wasSubmitted = false;

    bool onClick(Dialog::Button::ID btnID, gui::Button* pButton) override
    {
        if (btnID == Dialog::Button::ID::OK)
        {
            _answeredCorrectly = _viewQuiz.isAnswerCorrect();
            _wasSubmitted = true;
            return true; // Close dialog
        }
        return true;
    }

public:
    DialogQuiz(gui::Frame* pFrame, td::UINT4 wndID = 0)
        : gui::Dialog(pFrame,
            { {gui::Dialog::Button::ID::OK, tr("Submit"), gui::Button::Type::Default} },
            gui::Size(600, 400), wndID)
    {
        setTitle(tr("Mine Detected! Answer to Continue"));
        setCentralView(&_viewQuiz);
    }

    bool wasAnsweredCorrectly() const
    {
        return _answeredCorrectly;
    }

    bool wasSubmitted() const
    {
        return _wasSubmitted;
    }

    // Method to show dialog and wait for result
    bool showAndGetResult()
    {
        open();
        // Dialog is now open and will close when user clicks Submit
        // The onClick handler will set _answeredCorrectly
        return _answeredCorrectly;
    }
};
#pragma once
#include <gui/StatusBar.h>
#include <gui/Label.h>
#include <gui/HorizontalLayout.h>
#include <gui/Font.h>

class StatusBar : public gui::StatusBar
{
protected:
    gui::Label _lblMessage;
    gui::Label _lblGold;
    gui::Label _lblSteps;
    gui::Label _lblSize;
    
    gui::Label _noOfGold;
    gui::Label _noOfSteps;
    gui::Label _fieldInfo;
    
public:
    StatusBar()
    : gui::StatusBar(10)
    , _lblMessage("")
    , _lblGold("Gold:")
    , _lblSteps("Steps:")
    , _lblSize("Size:")
    , _noOfGold("0")
    , _noOfSteps("0")
    , _fieldInfo("00x00")
    {
        _lblMessage.setFont(gui::Font::ID::SystemBoldItalic);
        _lblMessage.setResizable();
        
        _noOfGold.disableRemeasuring();
        _noOfSteps.disableRemeasuring();
        _fieldInfo.disableRemeasuring();
        
        setMargins(4, 0, 4, 4);
        
        _layout.setSpaceBetweenCells(0);
        _layout << _lblMessage;
        _layout.appendSpace(4);
        _layout << _lblGold << _noOfGold;
        _layout.appendSpace(4);
        _layout << _lblSteps << _noOfSteps;
        _layout.appendSpace(4);
        _layout << _lblSize << _fieldInfo;
        setLayout(&_layout);
    }
    
    void setNoOfGold(int nGold)
    {
        td::String str;
        str.format("%d", nGold);
        _noOfGold.setTitle(str);
    }
    
    void setNoOfSteps(int nSteps)
    {
        td::String str;
        str.format("%d", nSteps);
        _noOfSteps.setTitle(str);
    }
    
    void setMessage(const td::String& msg)
    {
        _lblMessage.setTitle(msg);
    }
    
    void setFieldInfo(int w, int h)
    {
        td::String str;
        str.format("%dx%d", w, h);
        _fieldInfo.setTitle(str);
    }
};
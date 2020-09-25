#include "../../Includes/UserInterface/CheckBox.h"

#include <string>

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.cpp */

CheckBox::CheckBox(IWindow& aWindow, std::string_view aName, int aYCoord, int aXCoord, bool aChecked, bool aSelected) :
    mWindow(aWindow), mName(aName), mYCoord(aYCoord), mXCoord(aXCoord), mChecked(aChecked), mSelected(aSelected)
{}

void CheckBox::Draw()
{
    std::string lCheckBoxString{std::string("[") + (mChecked ? "X" : " ") + "]  " + mName};
    int         lColorPair{mSelected ? 7 : 1};
    mWindow.DrawString(mYCoord, mXCoord, lColorPair, lCheckBoxString);
}

bool CheckBox::DoAction()
{
    mChecked = !mChecked;
    return true;
}

void CheckBox::SetChecked(bool aChecked)
{
    mChecked = aChecked;
}

bool CheckBox::IsChecked() const
{
    return mChecked;
}

void CheckBox::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool CheckBox::IsSelected() const
{
    return mSelected;
}
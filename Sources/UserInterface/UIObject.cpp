#include "../../Includes/UserInterface/UIObject.h"

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.cpp */

UIObject::UIObject(IWindow& aWindow, std::string_view aName, int aYCoord, int aXCoord, bool aVisible, bool aSelectable) : mWindow(aWindow), mName(aName), mYCoord(aYCoord), mXCoord(aXCoord), mSelectable(aSelectable), mVisible(aVisible)
{
    
}

void UIObject::Move(int aYCoord, int aXCoord)
{
    mYCoord = aYCoord;
    mXCoord = aXCoord;
}

bool UIObject::DoAction()
{
  // Do nothing
  return true;
}

bool UIObject::IsSelected() const
{
    // Non selectable, not selected;
    return false;
}

void UIObject::SetSelected(bool /*aSelected*/)
{
    // By default an object is not selectable so ignore.
}

void UIObject::SetVisible(bool aVisible)
{
    mVisible = aVisible;
}

bool UIObject::IsVisible() const
{
    return mVisible;
}

IWindow& UIObject::GetWindow()
{
    return mWindow;
}

std::string_view UIObject::GetName()
{
    return mName;
}

bool UIObject::IsSelectable() const
{
    return mSelectable;
}

int UIObject::GetYCoord() const
{
    return mYCoord;
}

int UIObject::GetXCoord() const
{
    return mXCoord;
}
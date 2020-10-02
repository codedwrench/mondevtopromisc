#include "../../Includes/UserInterface/UIObject.h"

#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - UIObject.cpp */

UIObject::UIObject(IWindow&         aWindow,
                   std::string_view aName,
                   ScaleCalculation aCalculation,
                   const int&       aMaxHeight,
                   const int&       aMaxWidth,
                   bool             aVisible,
                   bool             aSelectable) :
    mWindow(aWindow),
    mName(aName), mSelectable(aSelectable), mVisible(aVisible), mYCoord(0), mXCoord(0), mScaleCalculation(aCalculation),
    mMaxHeight(aMaxHeight), mMaxWidth(aMaxWidth)
{
    Dimensions lParameters{mScaleCalculation(aMaxHeight, aMaxWidth)};
    mYCoord = lParameters.at(0);
    mXCoord = lParameters.at(1);
}

bool UIObject::DoAction()
{
    // Do nothing
    return true;
}

void UIObject::Scale()
{
    Dimensions lParameters{mScaleCalculation(mMaxHeight, mMaxWidth)};
    mYCoord = lParameters.at(0);
    mXCoord = lParameters.at(1);
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
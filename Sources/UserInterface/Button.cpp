#include "../../Includes/UserInterface/Button.h"

#include <string>
#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - Button.cpp */

Button::Button(IWindow&              aWindow,
               std::string_view      aName,
               ScaleCalculation      aCalculation,
               const int&            aMaxHeight,
               const int&            aMaxWidth,
               std::function<bool()> aAction,
               bool                  aSelected,
               bool                  aChecked,
               bool                  aVisible,
               bool                  aSelectable) :
    UIObject(aWindow, aName, aCalculation, aMaxHeight, aMaxWidth, aVisible, aSelectable),
    mChecked(aChecked), mSelected(aSelected), mAction{std::move(aAction)}
{}

void Button::Draw()
{
    if (IsVisible()) {
        std::string lButtonString{std::string("[ ") + GetName().data() + " ]"};
        int         lColorPair{mSelected ? 7 : 1};
        GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lButtonString);
    }
}

bool Button::DoAction()
{
    return mAction();
}

void Button::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool Button::IsSelected() const
{
    return mSelected;
}
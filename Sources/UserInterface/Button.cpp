#include "../../Includes/UserInterface/Button.h"

#include <string>
#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - Button.cpp */

Button::Button(IWindow&                    aWindow,
               std::string_view            aName,
               std::function<Dimensions()> aCalculation,
               std::function<bool()>       aAction,
               bool                        aSelected,
               bool                        aVisible,
               bool                        aSelectable) :
    UIObject(aWindow, aName, aCalculation, aVisible, aSelectable),
    mSelected(aSelected), mAction{std::move(aAction)}
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
#include "../../Includes/UserInterface/CheckBox.h"

#include <string>

/* Copyright (c) 2020 [Rick de Bondt] - CheckBox.cpp */

CheckBox::CheckBox(IWindow&                    aWindow,
                   std::string_view            aName,
                   std::function<Dimensions()> aCalculation,
                   bool&                       aModelCheckBox,
                   bool                        aSelected,
                   bool                        aChecked,
                   bool                        aVisible,
                   bool                        aSelectable) :
    UIObject(aWindow, aName, aCalculation, aVisible, aSelectable),
    mChecked(aChecked), mSelected(aSelected), mModelCheckBox{aModelCheckBox}
{}

void CheckBox::Draw()
{
    std::string lCheckBoxString{std::string("[") + (mChecked ? std::string("X") : std::string(" ")) + "]  " +
                                GetName().data()};
    int         lColorPair{mSelected ? 7 : 1};
    GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lCheckBoxString);
}

bool CheckBox::DoAction()
{
    mChecked       = !mChecked;
    mModelCheckBox = mChecked;
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
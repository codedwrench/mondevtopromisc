#include "../../Includes/UserInterface/TextField.h"

#include <string>
#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - TextField.cpp */

TextField::TextField(IWindow&              aWindow,
               std::string_view            aName,
               std::function<Dimensions()> aCalculation,
               std::string&                aTextReference,
               int                         aLength,
               bool                        aSelected,
               bool                        aVisible,
               bool                        aSelectable) :
        UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
        mTextReference{aTextReference},  mSelected(aSelected), mLength{aLength}
{}

void TextField::Draw()
{
    if (IsVisible()) {
        std::string lStringToDraw{};

        lStringToDraw = mTextReference;
        lStringToDraw.resize(mLength, '_');

        std::string lTextFieldString{std::string(GetName().data()) + ": | " + lStringToDraw + " |"};
        int         lColorPair{mSelected ? 7 : 1};
        GetWindow().DrawString(GetYCoord(), GetXCoord(), lColorPair, lTextFieldString);
    }
}

bool TextField::DoAction()
{
    mTextReference = "";
    return true;
}

void TextField::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool TextField::IsSelected() const
{
    return mSelected;
}
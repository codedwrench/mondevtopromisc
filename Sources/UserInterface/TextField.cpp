#include "../../Includes/UserInterface/TextField.h"

#include <string>
#include <utility>

/* Copyright (c) 2020 [Rick de Bondt] - TextField.cpp */

TextField::TextField(IWindow&                    aWindow,
                     std::string_view            aName,
                     std::function<Dimensions()> aCalculation,
                     std::string&                aTextReference,
                     int                         aLength,
                     bool                        aAcceptNumbers,
                     bool                        aAcceptLetters,
                     std::vector<char>           aAcceptSymbols,
                     bool                        aSelected,
                     bool                        aVisible,
                     bool                        aSelectable) :
    UIObject(aWindow, aName, std::move(aCalculation), aVisible, aSelectable),
    mTextReference{aTextReference}, mAcceptNumbers(aAcceptNumbers),
    mAcceptLetters(aAcceptLetters), mAcceptSymbols{std::move(aAcceptSymbols)}, mSelected(aSelected), mLength{aLength}
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

bool TextField::AddToText(char aCharacter)
{
    bool lReturn{false};

    if (mTextReference.length() < mLength) {
        mTextReference += aCharacter;
        lReturn = true;
    }

    return lReturn;
}

bool TextField::RemoveCharacter()
{
    bool lReturn{false};

    if (mTextReference.length() > 0) {
        mTextReference.resize(mTextReference.length() - 1);
    }

    return lReturn;
}

bool TextField::HandleKey(unsigned int aKeyCode)
{
    bool lReturn{false};

    if ((mAcceptLetters && (aKeyCode >= 'a' && aKeyCode <= 'z') || (aKeyCode >= 'A' && aKeyCode <= 'Z')) ||
        (mAcceptNumbers && (aKeyCode >= '0' && aKeyCode <= '9')) ||
        (find(mAcceptSymbols.begin(), mAcceptSymbols.end(), aKeyCode) != mAcceptSymbols.end())) {
        lReturn = AddToText(static_cast<char>(aKeyCode));
    } else if (aKeyCode == KEY_BACKSPACE || aKeyCode == 127 || aKeyCode == '\b') {
        lReturn = RemoveCharacter();
    }

    return lReturn;
}

void TextField::SetSelected(bool aSelected)
{
    mSelected = aSelected;
}

bool TextField::IsSelected() const
{
    return mSelected;
}
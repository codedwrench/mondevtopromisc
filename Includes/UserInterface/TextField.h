#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - TextField.h
 *
 * This file contains an class for a userinterface textfield.
 *
 **/

#include <functional>

#include "UIObject.h"

/**
 * Class for a userinterface TextField.
 */
class TextField : public UIObject
{
public:
    TextField(IWindow&                    aWindow,
              std::string_view            aName,
              std::function<Dimensions()> aCalculation,
              std::string&                aTextReference,
              int                         aLength,
              bool                        aAcceptNumbers = true,
              bool                        aAcceptLetters = true,
              std::vector<char>           aAcceptSymbols = {'.', ' '},
              bool                        aSelected      = false,
              bool                        aVisible       = true,
              bool                        aSelectable    = true);

    void Draw() override;
    bool HandleKey(unsigned int aKeyCode) override;

    void               SetSelected(bool aSelected) override;
    [[nodiscard]] bool IsSelected() const override;

private:
    bool AddToText(char aCharacter);
    bool RemoveCharacter();

    std::string&      mTextReference;
    int               mLength;
    bool              mAcceptLetters;
    bool              mAcceptNumbers;
    std::vector<char> mAcceptSymbols;
    bool              mSelected;
};

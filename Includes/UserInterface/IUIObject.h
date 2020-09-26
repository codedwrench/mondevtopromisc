#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IUIObject.h
 *
 * This file contains an interface for a userinterface object.
 *
 **/
class IUIObject
{
public:
    /**
     * Draws object on screen.
     */
    virtual void Draw() = 0;

    /**
     * Sets whether object is selected.
     * @param aSelected - Whether the object should be selected or not.
     **/
    virtual void SetSelected(bool aSelected) = 0;
    
    /**
     * Gets whether object is selectable.
     * @return true if object is selectable.
     **/
    [[nodiscard]] virtual bool IsSelectable() const = 0;

    /**
     * Gets whether object is selected.
     * @return true if object is selected.
     **/
    [[nodiscard]] virtual bool IsSelected() const = 0;

    /**
     * Does action corrosponding with object type.
     * @return true when successful.
     */
    virtual bool DoAction() = 0;

    /**
     * Moves object to desired position in the window.
     * Note: Does not move selection order, so beware of the user experience there.
     * @param aYCoord - The Y coordinate to move the object to.
     * @param aXCoord - The X coordinate to move the object to.
     **/
    virtual void Move(int aYCoord, int aXCoord) = 0;
};
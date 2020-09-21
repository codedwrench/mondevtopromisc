#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IWindow.h
 *
 * This file contains an interface for a userinterface window.
 *
 **/

#include <memory>

#include "IUIObject.h"

/**
 * Interface for Window classes.
 */
class IWindow
{
public:
    /**
     * Draws window on screen.
     */
    virtual void Draw() = 0;

    /**
     * Add objects to window.
     */
    virtual void AddObject(std::unique_ptr<IUIObject> aObject) = 0;

    /**
     * Refreshes window.
     */
    virtual void Refresh() = 0;

    /**
     * Moves window. Note: You cannot move a Window beyond the screen boundaries.
     * @param aYCoord - Y coordinate to move window to.
     * @param aXCoord - X coordinate to move window to.
     * @return true if successful.
     */
    virtual bool Move(int aYCoord, int aXcoord) = 0;

    /**
     * Resizes window. Note: You cannot resize a Window beyond the screen boundaries.
     * @param aLines - Amount of lines to resize the window to.
     * @param aColumns - Amount of columns to resize the window to.
     * @return true if successful.
     */
    virtual bool Resize(int aLines, int aColumns) = 0;

    /**
     * Advances cursor position to next selection.
     * @return true if successful.
     */
    virtual bool AdvanceSelection() = 0;

    /**
     * Recedes cursor position to previous selection.
     * @return true if successful.
     */
    virtual bool RecedeSelection() = 0;

    /**
     * Do action for object at selection.
     * @return true if successful.
     */
    virtual bool DoSelection() = 0;

    /**
     * Makes a window exclusive, other windows stop being processed.
     * @param aExclusive - Wether the window should be exclusive or not.
     */
    virtual void SetExclusive(bool aExclusive) = 0;

    /**
     * Gets if window is exclusive.
     * @return true if exclusive.
     */
    virtual bool IsExclusive() = 0;

    /**
     * Makes a window visible.
     * @param aVisible - Wether the window should be visible or not.
     */
    virtual void SetVisible(bool aVisible) = 0;

    /**
     * Gets if window is visible.
     * @return true if visible.
     */
    virtual bool IsVisible() = 0;
};

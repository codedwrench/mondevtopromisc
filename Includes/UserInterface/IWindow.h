#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IWindow.h
 *
 * This file contains an interface for a userinterface window.
 *
 **/

#include <functional>
#include <memory>
#include <string_view>

#include "IUIObject.h"

/**
 * Interface for Window classes.
 */
class IWindow
{
public:
    /**
     * Sets the window up.
     */
    virtual void SetUp() = 0;

    /**
     * Draws window on screen.
     */
    virtual void Draw() = 0;

    /**
     * Add objects to window.
     */
    virtual void AddObject(std::unique_ptr<IUIObject> aObject) = 0;

    /**
     * Draws a string on the Window at specified position.
     *
     * @param aYCoord - Y coordinate to draw the window at.
     * @param aXCoord - X coordinate to draw the window at.
     * @param aColorPair - Color to draw the string with.
     * @param aString - String to draw on the screen.
     */
    virtual void DrawString(int aYCoord, int aXCoord, int aColorPair, std::string_view aString) = 0;

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
    virtual bool Move(int aYCoord, int aXCoord) = 0;

    /**
     * Gets the size of the window.
     * @return pair with size of the window in order height, width.
     **/
    virtual std::pair<int, int> GetSize() = 0;

    /**
     * Resizes window. Note: You cannot resize a Window beyond the screen boundaries.
     * @param aLines - Amount of lines to resize the window to.
     * @param aColumns - Amount of columns to resize the window to.
     * @return true if successful.
     */
    virtual bool Resize(int aLines, int aColumns) = 0;

    /**
     * Scales window using size hints built into window.
     * @return true if successful.
     */
    virtual bool Scale() = 0;

    /**
     * Advances cursor position to next selection vertically.
     * @return true if successful.
     */
    virtual bool AdvanceSelectionVertical() = 0;

    /**
     * Recedes cursor position to previous selection vertically.
     * @return true if successful.
     */
    virtual bool RecedeSelectionVertical() = 0;

    /**
     * Advances cursor position to next selection horizontally.
     * @return true if successful.
     */
    virtual bool AdvanceSelectionHorizontal() = 0;

    /**
     * Recedes cursor position to previous selection horizontally.
     * @return true if successful.
     */
    virtual bool RecedeSelectionHorizontal() = 0;

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

    /**
     * Clears a line, up to length.
     * @param aYCoord - Y coord to clear the line at.
     * @param aLength - Length to clear.
     */
    virtual void ClearLine(int aYCoord, int aLength) = 0;

    /**
     * Clears window.
     */
    virtual void ClearWindow() = 0;
};

#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - IWindow.h
 *
 * This file contains an interface for a userinterface window.
 *
 **/

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
    virtual void AddObject() = 0;

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
    virtual bool Move(unsigned int aYCoord, unsigned int aXcoord) = 0;

    /**
     * Resizes window. Note: You cannot resize a Window beyond the screen boundaries.
     * @param aLines - Amount of lines to resize the window to.
     * @param aColumns - Amount of columns to resize the window to.
     * @return true if successful.
     */
    virtual bool Resize(unsigned int aLines, unsigned int aColumns) = 0;

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
};

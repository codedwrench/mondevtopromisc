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
     * Sets the object visible or not.
     * @param aVisible whether the object should be visible
     */
    virtual void SetVisible(bool aVisible) = 0;

    /**
     * Returns if the object is visible.
     * @return true if visible.
     */
    [[nodiscard]] virtual bool IsVisible() const = 0;

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
     * Scales object according to given calculation.
     **/
    virtual void Scale() = 0;

    /**
     * Gets Y coordinate of object.
     * @return Y coordinate of object.
     */
    [[nodiscard]] virtual int GetYCoord() const = 0;

    /**
     * Gets X coordinate of object.
     * @return X coordinate of object.
     */
    [[nodiscard]] virtual int GetXCoord() const = 0;

    /**
     * Gets name of object.
     * @return Name of object.
     */
    virtual std::string_view GetName() = 0;
};
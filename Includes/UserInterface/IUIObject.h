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
     * Does action corrosponding with object type.
     * @return true when successful.
     */
    virtual bool DoAction() = 0;
};
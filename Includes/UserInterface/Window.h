#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - Window.h
 *
 * This file contains an class for a userinterface window.
 *
 **/

#include <functional>
#include <memory>
#include <string>

#include <curses.h>
#undef timeout

#include "IWindow.h"

namespace Window_Constants
{
    using Dimensions       = std::array<int, 4>;
    using ScaleCalculation = std::function<Dimensions(const int&, const int&)>;
    using NCursesWindow    = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>;
    using ObjectList       = std::vector<std::shared_ptr<IUIObject>>;
}  // namespace Window_Constants

using namespace Window_Constants;

/**
 * Class with basic functions to setup a window.
 */
class Window : public IWindow
{
public:
    /**
     * @param aCalculation - The calculation that needs to be done to get to the scaling values
     * (takes height, width; returns starty, startx, height, width).
     **/
    Window(WindowModel&                       aModel,
           std::string_view                   aTitle,
           const std::function<Dimensions()>& aCalculation,
           bool                               aDrawBorder = true,
           bool                               aExclusive  = false,
           bool                               aVisible    = true);

    Window(const Window& aWindow) = delete;
    Window& operator=(const Window& aWindow) = delete;

    Window& operator=(Window&& aWindow) = delete;
    Window(Window&& aWindow)            = default;

    ~Window() = default;

    void SetUp() override;

    void ClearLine(int aYCoord, int aLength) override;

    void ClearWindow() override;

    void Draw() override;

    void DrawString(int aYCoord, int aXCoord, int aColorPair, std::string_view aString) override;

    void AddObject(std::shared_ptr<IUIObject> aObject) override;

    void Refresh() override;

    bool Move(int aYCoord, int aXCoord) override;

    WindowModel& GetModel() override;

    std::pair<int, int> GetSize() override;

    bool Resize(int aLines, int aColumns) override;

    bool Scale() override;

    bool SetSelection(int aSelection);

    bool AdvanceSelectionVertical() override;

    bool RecedeSelectionVertical() override;

    bool AdvanceSelectionHorizontal() override;

    bool RecedeSelectionHorizontal() override;

    void DeSelect() override;

    void SetExclusive(bool aExclusive) override;

    bool IsExclusive() override;

    void SetVisible(bool aVisible) override;

    bool IsVisible() override;

    bool HandleKey(unsigned int aKeyCode) override;

protected:
    [[nodiscard]] ObjectList& GetObjects();
    [[nodiscard]] int         GetSelectedObject() const;
    [[nodiscard]] const int&  GetHeightReference() const;
    [[nodiscard]] const int&  GetWidthReference() const;

private:
    WindowModel&                mModel;
    NCursesWindow               mNCursesWindow;
    std::string                 mTitle;
    std::function<Dimensions()> mScaleCalculation;
    int                         mHeight;
    int                         mWidth;
    bool                        mDrawBorder;
    bool                        mExclusive;
    bool                        mVisible;
    int                         mSelectedObject;
    ObjectList                  mObjects;
};

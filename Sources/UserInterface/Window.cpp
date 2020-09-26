#include "../../Includes/UserInterface/Window.h"

/* Copyright (c) 2020 [Rick de Bondt] - Window.cpp */

Window::Window(
    std::string_view aTitle, int aYCoord, int aXCoord, int aLines, int aColumns, bool aExclusive, bool aVisible) :
    mTitle{aTitle}, mLines{aLines}, mColumns{aColumns}, mDimensionsChanged{false},
    mNCursesWindow{nullptr}, mExclusive{aExclusive}, mVisible{aVisible}, mObjects{}
{
    mNCursesWindow = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(aLines, aColumns, aYCoord, aXCoord),
                                                                           [](WINDOW* aWin) { delwin(aWin); });
    SetUp();
}

void Window::SetUp()
{
    // Base window has no setup yet.
}

void Window::Draw()
{
    box(mNCursesWindow.get(), 0, 0);
    DrawString(0, 0, 7, mTitle);

    for (auto& lObject : mObjects) {
        lObject->Draw();
    }

    Refresh();
}

void Window::DrawString(int aYCoord, int aXCoord, int aColorPair, std::string_view aString)
{
    wattrset(mNCursesWindow.get(), COLOR_PAIR(aColorPair));
    mvwaddstr(mNCursesWindow.get(), aYCoord, aXCoord, aString.data());
    wattrset(mNCursesWindow.get(), COLOR_PAIR(1));
}

void Window::AddObject(std::unique_ptr<IUIObject> aObject)
{
    mObjects.push_back(std::move(aObject));
}

void Window::Refresh()
{
    wrefresh(mNCursesWindow.get());
}

bool Window::Move(int aYCoord, int aXCoord)
{
    return wmove(mNCursesWindow.get(), aYCoord, aXCoord) != ERR;
}

std::pair<int, int> Window::GetSize()
{
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mNCursesWindow.get(), lHeight, lWidth);
    if ((lWidth != mColumns) || (lHeight != mLines))
    {
        mColumns = lWidth;
        mLines = lHeight;
        mDimensionsChanged = true;
    }

    return {lHeight, lWidth};
}

bool Window::Scale(int aMaxHeight, int aMaxWidth)
{
  // No scale hints for base window.
}

bool Window::Resize(int aLines, int aColumns)
{
    return wresize(mNCursesWindow.get(), aLines, aColumns) == OK;
}

bool Window::AdvanceSelectionVertical()
{
    return false;
}

bool Window::RecedeSelectionVertical()
{
    return false;
}

bool Window::AdvanceSelectionHorizontal()
{
    return false;
}

bool Window::RecedeSelectionHorizontal()
{
    return false;
}

bool Window::DoSelection()
{
    return false;
}

bool Window::IsExclusive()
{
    return mExclusive;
}

void Window::SetExclusive(bool aExclusive)
{
    mExclusive = aExclusive;
}

bool Window::IsVisible()
{
    return mVisible;
}

void Window::SetVisible(bool aVisible)
{
    mVisible = aVisible;
}
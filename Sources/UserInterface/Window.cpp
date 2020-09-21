#include "../../Includes/UserInterface/Window.h"

/* Copyright (c) 2020 [Rick de Bondt] - Window.cpp */

Window::Window(
    std::string_view aTitle, int aYCoord, int aXCoord, int aLines, int aColumns, bool aExclusive, bool aVisible) :
    mTitle{aTitle},
    mNCursesWindow{nullptr}, mExclusive{aExclusive}, mVisible{aVisible}, mObjects{}
{
    mNCursesWindow = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(aLines, aColumns, aYCoord, aXCoord),
                                                                           [](WINDOW* aWin) { delwin(aWin); });
}

void Window::Draw()
{
    box(mNCursesWindow.get(), 0, 0);
    wattrset(mNCursesWindow.get(), COLOR_PAIR(7));
    mvwaddstr(mNCursesWindow.get(), 0, 0, mTitle.c_str());
    wattrset(mNCursesWindow.get(), COLOR_PAIR(1));
    Refresh();
}

void Window::AddObject(std::unique_ptr<IUIObject> aObject)
{
    mObjects.push_back(std::move(aObject));
}

void Window::Refresh()
{
    wrefresh(mNCursesWindow.get());
}

bool Window::Move(int aYCoord, int aXcoord)
{
    return false;
}

bool Window::Resize(int aLines, int aColumns)
{
    return false;
}

bool Window::AdvanceSelection()
{
    return false;
}

bool Window::RecedeSelection()
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
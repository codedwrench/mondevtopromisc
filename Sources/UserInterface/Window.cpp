#include "../../Includes/UserInterface/Window.h"

#include <iostream>

#include "../../Includes/Logger.h"


/* Copyright (c) 2020 [Rick de Bondt] - Window.cpp */

Window::Window(std::string_view                                                 aTitle,
               const std::function<std::array<int, 4>(const int&, const int&)>& aCalculation,
               const int&                                                       aMaxHeight,
               const int&                                                       aMaxWidth,
               bool                                                             aDrawBorder,
               bool                                                             aExclusive,
               bool                                                             aVisible) :
    mTitle{aTitle},
    mScaleCalculation(aCalculation), mMaxHeight(aMaxHeight),
    mMaxWidth(aMaxWidth), mNCursesWindow{nullptr}, mHeight{0}, mWidth{0},
    mDrawBorder(aDrawBorder), mExclusive{aExclusive}, mVisible{aVisible}, mSelectedObject{0}, mObjects{}
{
    std::array<int, 4> lWindowParameters{aCalculation(aMaxHeight, aMaxWidth)};
    mWidth         = aMaxWidth;
    mHeight        = aMaxHeight;
    mNCursesWindow = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(
        newwin(lWindowParameters.at(2), lWindowParameters.at(3), lWindowParameters.at(0), lWindowParameters.at(1)),
        [](WINDOW* aWin) { delwin(aWin); });
    SetUp();
}

void Window::SetUp()
{
    // Base window has no setup yet.
}

void Window::Draw()
{
    if (mDrawBorder) {
        box(mNCursesWindow.get(), 0, 0);
        DrawString(0, 0, 7, mTitle);
    }

    for (auto& lObject : mObjects) {
        lObject->Draw();
    }

    Refresh();
}

void Window::ClearLine(int aYCoord, int aLength)
{
    std::string lEmptySpace;
    lEmptySpace.resize(aLength, ' ');
    DrawString(aYCoord, 0, 1, lEmptySpace);
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
    bool lReturn{true};

    if (mvwin(mNCursesWindow.get(), aYCoord, aXCoord) == ERR) {
        Logger::GetInstance().Log("Could not move window to desired spot", Logger::Level::TRACE);
        lReturn = false;
    }

    return lReturn;
}

std::pair<int, int> Window::GetSize()
{
    int lHeight{0};
    int lWidth{0};
    getmaxyx(mNCursesWindow.get(), lHeight, lWidth);

    mHeight = lHeight;
    mWidth  = lWidth;

    return {lHeight, lWidth};
}

bool Window::Scale()
{
    bool               lReturn{false};
    std::array<int, 4> lParameters{mScaleCalculation(mMaxHeight, mMaxWidth)};
    if (Resize(lParameters.at(2), lParameters.at(3))) {
        if (Move(lParameters.at(0), lParameters.at(1))) {
            lReturn = true;
        }

        mHeight = lParameters.at(2);
        mWidth  = lParameters.at(3);

        for (auto& lObject : mObjects) {
            lObject->Scale();
        }
    }
    return lReturn;
}

bool Window::Resize(int aLines, int aColumns)
{
    bool lReturn{true};

    if (wresize(mNCursesWindow.get(), aLines, aColumns) == ERR) {
        Logger::GetInstance().Log("Could not resize window to desired size", Logger::Level::TRACE);
        lReturn = false;
    } else {
        // Clear window as to not have artifacts.
        ClearWindow();
    }

    return lReturn;
}

bool Window::AdvanceSelectionVertical()
{
    bool lReturn{false};
    bool lEndLoop{false};
    int  lIndex{mSelectedObject};
    while (!lEndLoop) {
        lIndex++;
        if (lIndex < mObjects.size()) {
            if (mObjects.at(lIndex)->IsVisible() && mObjects.at(lIndex)->IsSelectable()) {
                if (mSelectedObject >= 0 && mSelectedObject < mObjects.size()) {
                    mObjects.at(mSelectedObject)->SetSelected(false);
                }
                mSelectedObject = lIndex;
                mObjects.at(mSelectedObject)->SetSelected(true);
                lEndLoop = true;
                lReturn  = true;
            }
        } else {
            lEndLoop = true;
        }
    }
    return lReturn;
}

bool Window::RecedeSelectionVertical()
{
    bool lReturn{false};
    bool lEndLoop{false};
    int  lIndex{mSelectedObject};
    while (!lEndLoop) {
        lIndex--;
        if (lIndex >= 0) {
            if (mObjects.at(lIndex)->IsVisible() && mObjects.at(lIndex)->IsSelectable()) {
                if (mSelectedObject >= 0 && mSelectedObject < mObjects.size()) {
                    mObjects.at(mSelectedObject)->SetSelected(false);
                }
                mSelectedObject = lIndex;
                mObjects.at(mSelectedObject)->SetSelected(true);
                lEndLoop = true;
                lReturn  = true;
            }
        } else {
            lEndLoop = true;
        }
    }
    return lReturn;
}

bool Window::AdvanceSelectionHorizontal()
{
    return false;
}

bool Window::RecedeSelectionHorizontal()
{
    return false;
}

void Window::DeSelect()
{
    mSelectedObject = -1;
    for (auto& lObject : mObjects) {
        lObject->SetSelected(false);
    }
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

void Window::ClearWindow()
{
    std::pair<int, int> lSize{GetSize()};

    for (int lCount = 0; lCount < lSize.first; lCount++) {
        ClearLine(lCount, lSize.second);
    }
}

const int& Window::GetHeightReference() const
{
    return mHeight;
}

const int& Window::GetWidthReference() const
{
    return mWidth;
}
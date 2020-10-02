#include "../../Includes/UserInterface/Window.h"

#include <iostream>

#include "../../Includes/Logger.h"

/* Copyright (c) 2020 [Rick de Bondt] - Window.cpp */

Window::Window(WindowModel&                       aModel,
               std::string_view                   aTitle,
               const std::function<Dimensions()>& aCalculation,
               bool                               aDrawBorder,
               bool                               aExclusive,
               bool                               aVisible) :
    mModel{aModel},
    mTitle{aTitle}, mScaleCalculation(aCalculation), mNCursesWindow{nullptr}, mHeight{0}, mWidth{0},
    mDrawBorder(aDrawBorder), mExclusive{aExclusive}, mVisible{aVisible}, mSelectedObject{0}, mObjects{}
{
    Dimensions lWindowParameters{aCalculation()};
    mHeight        = lWindowParameters.at(0);
    mWidth         = lWindowParameters.at(1);
    mNCursesWindow = NCursesWindow(
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

WindowModel& Window::GetModel()
{
    return mModel;
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
    bool       lReturn{false};
    Dimensions lParameters{mScaleCalculation()};

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
    int  lCounter{0};
    if (mSelectedObject >= 0) {
        int lSelectionIndex{mObjects.at(mSelectedObject)->GetYCoord()};
        int lLowestHigherSelection{std::numeric_limits<int>::max()};
        int lLowestHigherSelectionYCoord{std::numeric_limits<int>::max()};

        for (auto& lObject : mObjects) {
            int lYCoord{lObject->GetYCoord()};
            if (lObject->IsVisible() && lObject->IsSelectable() && lYCoord > lSelectionIndex &&
                lYCoord < lLowestHigherSelectionYCoord) {
                lLowestHigherSelection       = lCounter;
                lLowestHigherSelectionYCoord = mObjects.at(lLowestHigherSelection)->GetYCoord();
            }
            lCounter++;
        }

        if (lLowestHigherSelection != std::numeric_limits<int>::max()) {
            mObjects.at(mSelectedObject)->SetSelected(false);
            mObjects.at(lLowestHigherSelection)->SetSelected(true);
            mSelectedObject = lLowestHigherSelection;
            lReturn         = true;
        }
    } else {
        mSelectedObject = 0;
        mObjects.at(0)->SetSelected(true);
        lReturn = true;
    }


    return lReturn;
}

bool Window::RecedeSelectionVertical()
{
    bool lReturn{false};
    int  lCounter{0};
    if (mSelectedObject >= 0) {
        int lSelectionIndex{mObjects.at(mSelectedObject)->GetYCoord()};
        int lHighestLowerSelection{-1};
        int lHighestLowerSelectionYCoord{-1};

        for (auto& lObject : mObjects) {
            int lYCoord{lObject->GetYCoord()};
            if (lObject->IsVisible() && lObject->IsSelectable() && lYCoord < lSelectionIndex &&
                lYCoord > lHighestLowerSelectionYCoord) {
                lHighestLowerSelection       = lCounter;
                lHighestLowerSelectionYCoord = mObjects.at(lHighestLowerSelection)->GetYCoord();
            }
            lCounter++;
        }

        if (lHighestLowerSelection != -1) {
            mObjects.at(mSelectedObject)->SetSelected(false);
            mObjects.at(lHighestLowerSelection)->SetSelected(true);
            mSelectedObject = lHighestLowerSelection;
            lReturn         = true;
        }
    } else {
        mSelectedObject = static_cast<int>(mObjects.size()) - 1;
        mObjects.at(mSelectedObject)->SetSelected(true);
        lReturn = true;
    }

    return lReturn;
}

bool Window::AdvanceSelectionHorizontal()
{
    bool lReturn{false};
    int  lCounter{0};
    if (mSelectedObject >= 0) {
        int lSelectionIndex{mObjects.at(mSelectedObject)->GetXCoord()};
        int lLowestHigherSelection{std::numeric_limits<int>::max()};
        int lLowestHigherSelectionXCoord{std::numeric_limits<int>::max()};

        for (auto& lObject : mObjects) {
            int lXCoord{lObject->GetXCoord()};
            if (lObject->IsVisible() && lObject->IsSelectable() && lXCoord > lSelectionIndex &&
                lXCoord < lLowestHigherSelectionXCoord) {
                lLowestHigherSelection       = lCounter;
                lLowestHigherSelectionXCoord = mObjects.at(lLowestHigherSelection)->GetXCoord();
            }
            lCounter++;
        }

        if (lLowestHigherSelection != std::numeric_limits<int>::max()) {
            mObjects.at(mSelectedObject)->SetSelected(false);
            mObjects.at(lLowestHigherSelection)->SetSelected(true);
            mSelectedObject = lLowestHigherSelection;
            lReturn         = true;
        }
    } else {
        mSelectedObject = 0;
        mObjects.at(0)->SetSelected(true);
        lReturn = true;
    }

    return lReturn;
}

bool Window::RecedeSelectionHorizontal()
{
    bool lReturn{false};
    int  lCounter{0};
    if (mSelectedObject >= 0) {
        int lSelectionIndex{mObjects.at(mSelectedObject)->GetXCoord()};
        int lHighestLowerSelection{-1};
        int lHighestLowerSelectionXCoord{-1};

        for (auto& lObject : mObjects) {
            int lXCoord{lObject->GetXCoord()};
            if (lObject->IsVisible() && lObject->IsSelectable() && lXCoord < lSelectionIndex &&
                lXCoord > lHighestLowerSelectionXCoord) {
                lHighestLowerSelection       = lCounter;
                lHighestLowerSelectionXCoord = mObjects.at(lHighestLowerSelection)->GetXCoord();
            }
            lCounter++;
        }

        if (lHighestLowerSelection != -1) {
            mObjects.at(mSelectedObject)->SetSelected(false);
            mObjects.at(lHighestLowerSelection)->SetSelected(true);
            mSelectedObject = lHighestLowerSelection;
            lReturn         = true;
        }
    } else {
        mSelectedObject = 0;
        mObjects.at(0)->SetSelected(true);
        lReturn = true;
    }

    return lReturn;
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
    bool lReturn{false};
    if ((mSelectedObject >= 0) && (mSelectedObject < mObjects.size()) && (mObjects.at(mSelectedObject)->IsVisible()) &&
        (mObjects.at(mSelectedObject)->IsSelectable())) {
        lReturn = mObjects.at(mSelectedObject)->DoAction();
    }
    return lReturn;
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

ObjectList& Window::GetObjects()
{
    return mObjects;
}

int Window::GetSelectedObject() const
{
    return mSelectedObject;
}
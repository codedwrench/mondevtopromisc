#include "../Includes/WindowModel.h"

/* Copyright (c) 2020 [Rick de Bondt] - WindowModel.cpp */

using namespace WindowModel_Constants;

std::string BoolToString(bool aBool)
{
    return aBool ? "true" : "false";
}

bool StringToBool(std::string_view aString)
{
    bool lReturn{false};

    if(aString == "true")
    {
        lReturn = true;
    }

    return lReturn;
}

bool WindowModel::SaveToFile(std::string_view aPath) const
{
    bool          lReturn{false};
    std::ofstream lFile;
    lFile.open(aPath.data());

    if (lFile.is_open() && lFile.good()) {
        lFile << cSaveLogLevel << ": \"" << Logger::ConvertLogLevelToString(mLogLevel) << "\"" << std::endl;
        lFile << cSaveAutoDiscoverPSPVita << ": " << BoolToString(mAutoDiscoverPSPVitaNetworks) << std::endl;
        lFile << cSaveAutoDiscoverXLinkKai << ": " << BoolToString(mAutoDiscoverXLinkKaiInstance) << std::endl;
        lFile << cSaveUseXLinkKaiHints << ": " << BoolToString(mXLinkKaiHints) << std::endl;
        lFile << cSaveWifiAdapter << ": \"" << mWifiAdapter << "\"" << std::endl;
        lFile << cSaveChannel << ": \"" << mChannel << "\"" << std::endl;
        lFile << cSaveXLinkIp << ": \"" << mXLinkIp << "\"" << std::endl;
        lFile << cSaveXLinkPort << ": \"" << mXLinkPort << "\"" << std::endl;
        lFile.close();

        if(lFile.good())
        {
            lFile.close();
            lReturn = true;
        } else {
            Logger::GetInstance().Log("Could not save config", Logger::Level::ERROR);
        }
    } else {
            Logger::GetInstance().Log(std::string("Could not open/create config file: ") + aPath.data(), Logger::Level::ERROR);
    }

    return lReturn;
}

bool WindowModel::LoadFromFile(std::string_view aPath)
{
    return false;
}

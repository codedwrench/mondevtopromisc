#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetConversionFunctions.h
 *
 * This file contains some general conversion functions for network related things.
 *
 **/

#if defined(_MSC_VER) or defined(__MINGW32__)
#include <stdlib.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#else
#include <byteswap.h>  // bswap_16 bswap_32 bswap_64
#endif

#include <ios>
#include <sstream>
#include <string>


/**
 * Converts a channel to frequency, only 2.4Ghz frequencies supported.
 * @param aChannel - Channel to convert,
 * @return Channel as frequency.
 */
static int ConvertChannelToFrequency(int aChannel)
{
    int lReturn{-1};

    // 2.4GHz, steps of 5hz.
    if (aChannel >= 1 && aChannel <= 13) {
        lReturn = 2412 + ((aChannel - 1) * 5);
    }

    return lReturn;
}

/**
 * Helper function to get raw data more easily.
 * @param aPacket - Packet to grab data from.
 */
template<typename Type> static Type GetRawData(std::string_view aPacket, unsigned int aIndex)
{
    return (*reinterpret_cast<const Type*>(aPacket.data() + aIndex));
}

/**
 * Helper function to get raw data as string more easily.
 */
static std::string GetRawString(std::string_view aPacket, unsigned int aIndex, unsigned int aLength)
{
    const char* lData{reinterpret_cast<const char*>(aPacket.data() + aIndex)};
    return std::string(lData, aLength);
}

/**
 * Converts a mac address string in format (xx:xx:xx:xx:xx:xx) to an int, has no safety build in for invalid
 * strings!
 * @param aMac - The mac address string to convert to an int.
 * @return int with the converted mac address.
 */
static uint64_t MacToInt(std::string_view aMac)
{
    uint64_t lResult{0};

    if (!aMac.empty()) {
        std::istringstream lStringStream(aMac.data());
        uint64_t           lNibble{0};
        lStringStream >> std::hex;
        while (lStringStream >> lNibble) {
            lResult = (lResult << 8) + lNibble;
            lStringStream.get();
        }
    }

    return lResult;
}

/**
 * Swaps endianness of Mac.
 * @param aMac - Mac to swap.
 * @return swapped mac.
 */
static uint64_t SwapMacEndian(uint64_t aMac)
{
    // Little- to Big endian
    aMac = bswap_64(aMac);
    return aMac >> 16U;
}

/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef FEATURE_GNSS_SUPPORT
#include "string_utils.h"

#include <codecvt>
#include <sstream>
#include <string_ex.h>

#include "cstddef"
#include "cstdint"
#include "cstdlib"
#include "ostream"
#include "string"
#include "vector"

namespace OHOS {
namespace Location {
static constexpr uint8_t HEX_OFFSET = 4;
static constexpr uint8_t STEP_2BIT = 2;
static constexpr uint8_t GSM_BIT = 7;
static constexpr char HEX_TABLE[] = "0123456789ABCDEF";

StringUtils::StringUtils() {}

StringUtils::~StringUtils() {}

uint16_t StringUtils::HexCharToInt(char c)
{
    const uint8_t decimal = 10;
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return (c - 'A' + decimal);
    }
    if (c >= 'a' && c <= 'f') {
        return (c - 'a' + decimal);
    }
    return 0;
}

std::string StringUtils::StringToHex(const std::string &data)
{
    std::stringstream ss;
    for (std::string::size_type i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const char *data, int byteLength)
{
    std::stringstream ss;
    for (int i = 0; i < byteLength; ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::StringToHex(const std::vector<uint8_t> &data)
{
    std::stringstream ss;
    for (std::size_t i = 0; i < data.size(); ++i) {
        unsigned char temp = static_cast<unsigned char>(data[i]) >> HEX_OFFSET;
        ss << HEX_TABLE[temp] << HEX_TABLE[static_cast<unsigned char>(data[i]) & 0xf];
    }
    return ss.str();
}

std::string StringUtils::HexToString(const std::string &str)
{
    std::string result;
    uint8_t hexDecimal = 16;
    uint8_t hexStep = 2;
    if (str.length() <= 0) {
        return result;
    }
    for (size_t i = 0; i < str.length() - 1; i += STEP_2BIT) {
        std::string byte = str.substr(i, hexStep);
        char chr = 0;
        long strTemp = strtol(byte.c_str(), nullptr, hexDecimal);
        if (strTemp > 0) {
            chr = static_cast<char>(strTemp);
        }
        result.push_back(chr);
    }
    return result;
}

std::vector<uint8_t> StringUtils::HexToByteVector(const std::string &str)
{
    std::vector<uint8_t> ret;
    int sz = static_cast<int>(str.length());
    if (sz <= 0) {
        return ret;
    }
    for (int i = 0; i < (sz - 1); i += STEP_2BIT) {
        auto temp = static_cast<uint8_t>((HexCharToInt(str.at(i)) << HEX_OFFSET) | HexCharToInt(str.at(i + 1)));
        ret.push_back(temp);
    }
    return ret;
}

std::string StringUtils::ToUtf8(const std::u16string &str16)
{
    if (str16.empty()) {
        std::string ret;
        return ret;
    }
    return Str16ToStr8(str16);
}

std::u16string StringUtils::ToUtf16(const std::string &str)
{
    if (str.empty()) {
        std::u16string ret;
        return ret;
    }
    return Str8ToStr16(str);
}

std::wstring StringUtils::Utf8ToWstring(const std::string& utf8Str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(utf8Str);
}

std::wstring StringUtils::Ucs2ToWstring(const std::string& ucs2Str)
{
    std::wstring_convert<std::codecvt_utf16<wchar_t>> converter;
    return converter.from_bytes(ucs2Str);
}

std::string StringUtils::WstringToString(const std::wstring& wStr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wStr);
}

std::string StringUtils::Gsm7Decode(const std::string &gsm7Str)
{
    const std::string gsm7Chars = "@£$¥èéùìòÇ\nØø\n\rÅåΔ_ΦΓΛΩΠΨΣΘΞ\x1bÆæßÉ !\"#¤%&'()*+,-./0123456789:;<=>?"
        "¡ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÑÜ`¿abcdefghijklmnopqrstuvwxyzäöñüà";
    const std::string extChar = "^{}\\[~]|€";

    std::string decodedString;
    std::bitset<GSM_BIT> bits(0);

    for (char c : gsm7Str) {
        unsigned char index = static_cast<unsigned char>(c);
        if (index < 0x20) {
            bits = (index == 0x1B) ? 0x1B : 0x5E;
        } else if (index < 0x60) {
            bits = gsm7Chars[index - 0x20];
        } else {
            bits = extChar[index - 0x60];
        }

        for (int i = 6; i >= 0; --i) {
            if (bits[i]) {
                bits <<= i + 1;
                decodedString += static_cast<char>(bits.to_ulong());
                bits = bits >> (i + 1);
                break;
            }
        }
    }

    return decodedString;
}

} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
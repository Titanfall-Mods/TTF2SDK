#include "stdafx.h"

namespace Util
{
    // Taken from https://stackoverflow.com/a/18374698
    std::wstring Widen(const std::string& input)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.from_bytes(input);
    }

    // Taken from https://stackoverflow.com/a/18374698
    std::string Narrow(const std::wstring& input)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.to_bytes(input);
    }

    // This will convert some data like "Hello World" to "48 65 6C 6C 6F 20 57 6F 72 6C 64"
    // Taken mostly from https://stackoverflow.com/a/3382894
    std::string DataToHex(const char* input, size_t len)
    {
        static const char* const lut = "0123456789ABCDEF";

        std::string output;
        output.reserve(2 * len);
        for (size_t i = 0; i < len; i++)
        {
            const unsigned char c = input[i];
            output.push_back(lut[c >> 4]);
            output.push_back(lut[c & 15]);
        }

        return output;
    }
}

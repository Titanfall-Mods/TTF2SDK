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
}

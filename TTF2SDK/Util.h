#pragma once

namespace Util
{
    std::wstring Widen(const std::string& input);
    std::string Narrow(const std::wstring& input);
}

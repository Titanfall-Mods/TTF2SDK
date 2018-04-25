#pragma once

namespace Util
{
    std::wstring Widen(const std::string& input);
    std::string Narrow(const std::wstring& input);
    std::string DataToHex(const char* input, size_t len);
    void SuspendAllOtherThreads();
    void ResumeAllOtherThreads();

    struct ThreadSuspender
    {
        ThreadSuspender();
        ~ThreadSuspender();
    };

    constexpr const char* GetContextName(ExecutionContext context)
    {
        if (context == CONTEXT_CLIENT)
        {
            return "CLIENT";
        }
        else if (context == CONTEXT_SERVER)
        {
            return "SERVER";
        }
        else
        {
            return "UNKNOWN";
        }
    }

    void FindAndReplaceAll(std::string& data, const std::string& search, const std::string& replace);
    void* ResolveLibraryFunction(const std::string& module, const std::string& funcName);
}

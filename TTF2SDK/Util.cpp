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

    template <DWORD(*Func)(HANDLE)>
    void PerformThreadOperation()
    {
        auto logger = spdlog::get("logger");

        // Take a snapshot of all running threads  
        HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnap == INVALID_HANDLE_VALUE)
        {
            logger->error("Failed to call CreateToolhelp32Snapshot, cannot perform thread operation");
            return;
        }

        // Fill in the size of the structure before using it. 
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        // Retrieve information about the first thread,
        // and exit if unsuccessful
        if (!Thread32First(hThreadSnap, &te32))
        {
            logger->error("Failed to call Thread32First, cannot perform thread operation");
            CloseHandle(hThreadSnap);
            return;
        }

        // Now walk the thread list of the system,
        // and display information about each thread
        // associated with the specified process
        do
        {
            if (te32.th32OwnerProcessID == GetCurrentProcessId() && te32.th32ThreadID != GetCurrentThreadId())
            {
                HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (thread != NULL)
                {
                    Func(thread);
                    CloseHandle(thread);
                }
            }
        }
        while (Thread32Next(hThreadSnap, &te32));

        CloseHandle(hThreadSnap);
    }

    void SuspendAllOtherThreads()
    {
        PerformThreadOperation<SuspendThread>();
    }

    void ResumeAllOtherThreads()
    {
        PerformThreadOperation<ResumeThread>();
    }
}

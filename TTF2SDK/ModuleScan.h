#pragma once

class SigScanException : public std::runtime_error
{
public:
    SigScanException(const std::string& errorStr)
        : std::runtime_error(errorStr) {}
};

class ModuleScan
{
private:
    char* m_moduleBase;
    DWORD m_moduleLen;
    HMODULE m_moduleHandle;

public:
    ModuleScan(const std::string& moduleName);
    ModuleScan(HMODULE moduleName);

    void* Scan(const char* sig, const char* mask, size_t sigLength);
    void* Scan(const char* sig, const char* mask);
};

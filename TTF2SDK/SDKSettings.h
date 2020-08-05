#pragma once

struct SDKSettings
{
    char* BasePath;
    bool DeveloperMode;

    SDKSettings()
    {
        BasePath = nullptr;
        DeveloperMode = false;
    }

    SDKSettings(const SDKSettings& other)
    {
        size_t size = strlen(other.BasePath) + 1;
        BasePath = new char[size];
        strcpy_s(BasePath, size, other.BasePath);
        DeveloperMode = other.DeveloperMode;
    }

    SDKSettings& operator=(SDKSettings tmp) {
        std::swap(BasePath, tmp.BasePath);
        DeveloperMode = tmp.DeveloperMode;
        return *this;
    }

    ~SDKSettings()
    {
        if (BasePath != nullptr)
        {
            delete BasePath;
            BasePath = nullptr;
        }
    }
};
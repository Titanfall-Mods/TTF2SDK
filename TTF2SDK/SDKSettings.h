#pragma once

struct SDKSettings
{
    char* BasePath;

    SDKSettings()
    {
        BasePath = nullptr;
    }

    SDKSettings(const SDKSettings& other)
    {
        size_t size = strlen(other.BasePath) + 1;
        BasePath = new char[size];
        strcpy_s(BasePath, size, other.BasePath);
    }

    SDKSettings& operator=(SDKSettings tmp) {
        std::swap(BasePath, tmp.BasePath);
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
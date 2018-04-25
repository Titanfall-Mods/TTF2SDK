#pragma once

class IEngineClient
{
public:
    struct VTable
    {
        void* unknown[5];
        int(*FirstFuncOfInterest) (IEngineClient* engineClient, const char* mapName, char a3);
        int(*SecondFuncOfInterest) (IEngineClient* engineClient, const char* mapName);
    };

    VTable* m_vtable;
};

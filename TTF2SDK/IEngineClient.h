#pragma once

class IEngineClient
{
public:
    struct VTable
    {
        void* unknown[5];
    };

    VTable* m_vtable;
};

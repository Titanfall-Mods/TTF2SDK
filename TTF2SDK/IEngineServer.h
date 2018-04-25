#pragma once

enum SpewType_t
{
    SPEW_MESSAGE = 0,
    SPEW_WARNING,
    SPEW_ASSERT,
    SPEW_ERROR,
    SPEW_LOG,

    SPEW_TYPE_COUNT
};

class IEngineServer
{
public:
    struct VTable
    {
        void* unknown[175];
        __int64(*SpewFunc) (IEngineServer* engineServer, SpewType_t type, const char* format, va_list va);
    };

    VTable* m_vtable;
};

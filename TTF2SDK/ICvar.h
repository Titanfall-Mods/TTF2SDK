#pragma once

class ConCommand;

class ICvar
{
public:
    struct VTable
    {
        void* unknown[10];
        void(*UnregisterConCommand) (ICvar* cvar, ConCommand* pCommandBase);
    };

    VTable* m_vtable;
};

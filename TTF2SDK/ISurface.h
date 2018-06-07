#pragma once

class ISurface
{
public:
    struct VTable
    {
        void* unknown1[61];
        void(*SetCursor) (ISurface* surface, unsigned int cursor);
        void* unknown2[12];
        void(*UnlockCursor) (ISurface* surface);
        void(*LockCursor) (ISurface* surface);
    };

    VTable* m_vtable;
};

#pragma once

struct model_t;

class IVModelInfo
{
public:
    struct VTable
    {
        void* dtor;
        model_t* (*GetModel) (IVModelInfo* modelInfo, int modelIndex);
        int (*GetModelIndex) (IVModelInfo* modelInfo, const char* modelName);
    };

    VTable* m_vtable;
};

#pragma once

typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef char SQChar;

typedef SQUnsignedInteger SQBool;
typedef SQInteger SQRESULT;

typedef struct SQVM* HSQUIRRELVM;
typedef void(*SQPRINTFUNCTION)(HSQUIRRELVM, const SQChar*, ...);

#pragma pack(push,1)
struct SQSharedState
{
    unsigned char unknownData[0x4350];
    SQPRINTFUNCTION _printfunc;
};

struct SQVM
{
    unsigned char unknownData[0x50];
    SQSharedState* _sharedstate;
};
#pragma pack(pop)

#define _ss(_vm_) (_vm_)->_sharedstate

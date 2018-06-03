#pragma once

#define SQ_SUCCEEDED(res) (res>=0)

#define SQTrue	(1)
#define SQFalse	(0)

typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef char SQChar;

typedef SQUnsignedInteger SQBool;
typedef SQInteger SQRESULT;

typedef struct SQVM* HSQUIRRELVM;
typedef void(*SQPRINTFUNCTION)(HSQUIRRELVM, const SQChar*, ...);
typedef SQInteger(*SQFUNCTION)(HSQUIRRELVM);

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

struct R2SquirrelVM
{
    unsigned char Data[8];
    HSQUIRRELVM sqvm;
};

struct SQFuncRegistration
{
    ExecutionContext context;
    std::string name;
    SQFUNCTION func;

    SQFuncRegistration(ExecutionContext context, const std::string& name, SQFUNCTION func)
        : context(context),
          name(name),
          func(func)
    { }
};

class SquirrelManager
{
private:
    std::shared_ptr<spdlog::logger> m_logger;
    R2SquirrelVM** m_ppClientVM = nullptr;
    R2SquirrelVM** m_ppServerVM = nullptr;
    std::vector<SQFuncRegistration> m_funcsToRegister;
    std::vector<std::string> m_clientCallbacks;
    std::vector<std::string> m_serverCallbacks;

public:
    SquirrelManager(ConCommandManager& conCommandManager);

    template<ExecutionContext context>
    SQInteger BasePrintHook(HSQUIRRELVM v);
    void PrintFunc(HSQUIRRELVM v, const SQChar* source, const SQChar* s, va_list args);

    HSQUIRRELVM GetClientSQVM();
    HSQUIRRELVM GetServerSQVM();

    template<ExecutionContext context>
    void CompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column);

    void RunServerCommand(const CCommand& args);
    void RunClientCommand(const CCommand& args);

    template<ExecutionContext context>
    void ExecuteCode(const char* code);

    template<ExecutionContext context>
    void RegisterFunction(HSQUIRRELVM v, const SQChar* name, SQFUNCTION func, int64_t unk1);

    void AddFuncRegistration(ExecutionContext context, const std::string& name, SQFUNCTION func);

    template<ExecutionContext context>
    R2SquirrelVM* CreateNewVMHook(int64_t a1, int a2, float a3);

    template<ExecutionContext context>
    int64_t RegisterMathlibHook(HSQUIRRELVM a1, int64_t a2, int64_t a3, int64_t a4);

    int64_t RunClientInitCallbacksHook();
    int64_t RunServerInitCallbacksHook();

    void AddServerCallback(const std::string& cb);
    void AddClientCallback(const std::string& cb);
    void ClearCallbacks();
};

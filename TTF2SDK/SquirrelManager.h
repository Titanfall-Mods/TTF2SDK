#pragma once

#define SQ_SUCCEEDED(res) (res>=0)

#define SQTrue	(1)
#define SQFalse	(0)

typedef float SQFloat;
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

struct SQFuncRegistrationInternal
{
    const char* squirrelFuncName;
    const char* cppFuncName;
    const char* helpText;
    const char* returnValueType;
    const char* argTypes;
    int16_t somethingThatsZero;
    int16_t padding1;
    int32_t unknown1;
    int64_t unknown2;
    int32_t unknown3;
    int32_t padding2;
    int64_t unknown4;
    int64_t unknown5;
    int64_t unknown6;
    int32_t unknown7;
    int32_t padding3;
    void* funcPtr;

    SQFuncRegistrationInternal()
    {
        memset(this, 0, sizeof(SQFuncRegistrationInternal));
        this->padding2 = 32;
    }
};

class SQFuncRegistration
{
public:
    SQFuncRegistration(
        ExecutionContext context,
        const std::string& returnType,
        const std::string& name,
        const std::string& argTypes,
        const std::string& helpText,
        SQFUNCTION funcPtr
    ) : 
        m_context(context),
        m_retValueType(returnType),
        m_funcName(name),
        m_argTypes(argTypes),
        m_helpText(helpText)
    {
        m_internalReg.squirrelFuncName = m_funcName.c_str();
        m_internalReg.cppFuncName = m_funcName.c_str();
        m_internalReg.helpText = m_helpText.c_str();
        m_internalReg.returnValueType = m_retValueType.c_str();
        m_internalReg.argTypes = m_argTypes.c_str();
        m_internalReg.funcPtr = funcPtr;
    }

    ExecutionContext GetContext() const
    {
        return m_context;
    }

    SQFuncRegistrationInternal* GetInternalReg()
    {
        return &m_internalReg;
    }

    const std::string& GetName() const
    {
        return m_funcName;
    }

private:
    ExecutionContext m_context;
    std::string m_funcName;
    std::string m_helpText;
    std::string m_retValueType;
    std::string m_argTypes;
    SQFuncRegistrationInternal m_internalReg;
};

class SquirrelManager
{
private:
    std::shared_ptr<spdlog::logger> m_logger;
    R2SquirrelVM** m_ppClientVM = nullptr;
    R2SquirrelVM** m_ppServerVM = nullptr;
    std::list<SQFuncRegistration> m_funcsToRegister;
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
    void ExecuteServerCode(const char* code);
    void ExecuteClientCode(const char* code);

    void RegisterFunction(R2SquirrelVM* vm, SQFuncRegistration& reg);

    void AddFuncRegistration(
        ExecutionContext context,
        const std::string& returnType,
        const std::string& name,
        const std::string& argTypes,
        const std::string& helpText,
        SQFUNCTION func
    );

    template<ExecutionContext context>
    R2SquirrelVM* CreateNewVMHook(int64_t a1, int a2, float a3);

    int64_t RunClientInitCallbacksHook();
    int64_t RunServerInitCallbacksHook();

    void AddServerCallback(const std::string& cb);
    void AddClientCallback(const std::string& cb);
    void ClearCallbacks();
};

extern SharedSigFunc<const SQChar*, HSQUIRRELVM, SQInteger> sq_getstring;
extern SharedSigFunc<SQInteger, HSQUIRRELVM, SQInteger> sq_getinteger;
extern SharedSigFunc<SQFloat, HSQUIRRELVM, SQInteger> sq_getfloat;

extern SharedSigFunc<void, HSQUIRRELVM, const SQChar*, SQInteger> sq_pushstring;
extern SharedSigFunc<void, HSQUIRRELVM, SQInteger> sq_pushinteger;
extern SharedSigFunc<void, HSQUIRRELVM, SQFloat> sq_pushfloat;

extern SharedSigFunc<void, HSQUIRRELVM, SQInteger> sq_newarray;
extern SharedSigFunc<SQRESULT, HSQUIRRELVM, SQInteger> sq_arrayappend;

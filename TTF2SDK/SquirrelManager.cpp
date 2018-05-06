#include "stdafx.h"

SquirrelManager& SQManager()
{
    return SDK().GetSQManager();
}

struct CompileBufferState
{
    const SQChar* buffer;
    const SQChar* bufferPlusLength;
    const SQChar* bufferAgain;

    CompileBufferState(const std::string& code)
    {
        buffer = code.c_str();
        bufferPlusLength = code.c_str() + code.size();
        bufferAgain = code.c_str();
    }
};

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&SquirrelManager::##name), &SquirrelManager::##name, decltype(&SQManager), &SQManager>::Call

SharedHookedFunc<SQInteger, HSQUIRRELVM> base_print("\x40\x53\x48\x83\xEC\x30\xBA\x00\x00\x00\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x8B\x53\x68", "xxxxxxx????xxxx????xxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, CompileBufferState*, const SQChar*, int, int> sq_compilebuffer("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x50\x41\x8B\xE9", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");
SharedSigFunc<void, HSQUIRRELVM> sq_pushroottable("\x8B\x51\x68\x44\x8B\xC2\x49\xC1\xE0\x04\x8D\x42\x01\x4C\x03\x41\x70\x89\x41\x68\xF7\x81\xB8", "xxxxxxxxxxxxxxxxxxxxxxx");
SharedSigFunc<SQRESULT, HSQUIRRELVM, SQInteger, SQBool, SQBool> sq_call("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xF2", "xxxxxx?xxx?xxx?xxxxxxxx?xxx");
SharedHookedFunc<void, HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger, SQInteger> sqstd_compiler_error("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x41\x50", "xxxx?xxxx?xxxx?xxxx?xxxxx????xxxx");

SigScanFunc<void> clientVMFinder("client.dll", "\x44\x8B\xC2\x48\x8B\xD1\x48\x8B\x0D\x00\x00\x00\x00", "xxxxxxxxx????");
SigScanFunc<void> serverVMFinder("server.dll", "\x48\x89\x5C\x24\x00\x55\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xEA", "xxxx?xxxx?xxx????xxx");

SquirrelManager::SquirrelManager(ConCommandManager& conCommandManager)
{
    m_logger = spdlog::get("logger");

    // TODO: maybe implement some kind of checking to make sure these pointers seem legit

    // Resolve pointer to VM structure in client.dll
    char* funcBase = (char*)clientVMFinder.GetFuncPtr();
    int offset = *(int*)(funcBase + 9);
    m_ppClientVM = (R2SquirrelVM**)(funcBase + 13 + offset);

    // Resolve pointer to VM structure in server.dll
    funcBase = (char*)serverVMFinder.GetFuncPtr();
    offset = *(int*)(funcBase + 13);
    m_ppServerVM = (R2SquirrelVM**)(funcBase + 17 + offset);

    base_print.Hook(WRAPPED_MEMBER(BasePrintHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(BasePrintHook<CONTEXT_SERVER>));
    sqstd_compiler_error.Hook(WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_CLIENT>), WRAPPED_MEMBER(CompilerErrorHook<CONTEXT_SERVER>));

    // Add concommands to run client and server code
    conCommandManager.RegisterCommand("run_server", WRAPPED_MEMBER(RunServerCommand), "Execute Squirrel code in server context", 0);
    conCommandManager.RegisterCommand("run_client", WRAPPED_MEMBER(RunClientCommand), "Execute Squirrel code in client context", 0);
}

template<ExecutionContext context>
SQInteger SquirrelManager::BasePrintHook(HSQUIRRELVM v)
{
    static auto printFuncLambda = [](HSQUIRRELVM v, const SQChar* s, ...) {
        va_list vl;
        va_start(vl, s);
        SQManager().PrintFunc(v, Util::GetContextName(context), s, vl);
        va_end(vl);
    };

    SQPRINTFUNCTION oldPrintFunc = _ss(v)->_printfunc;
    _ss(v)->_printfunc = printFuncLambda;
    SQInteger res = base_print.Call<context>(v);
    _ss(v)->_printfunc = oldPrintFunc;

    return res;
}

void SquirrelManager::PrintFunc(HSQUIRRELVM v, const SQChar* source, const SQChar* s, va_list args)
{
    SQChar buf[1024];

    int charactersWritten = _vsnprintf_s(buf, _TRUNCATE, s, args);
    if (charactersWritten > 0)
    {
        if (buf[charactersWritten - 1] == '\n')
        {
            buf[charactersWritten - 1] = 0;
        }

        m_logger->info("{}: {}", source, buf);
    }
}

HSQUIRRELVM SquirrelManager::GetClientSQVM()
{
    if (*m_ppClientVM != nullptr)
    {
        return (*m_ppClientVM)->sqvm;
    }

    return nullptr;
}

HSQUIRRELVM SquirrelManager::GetServerSQVM()
{
    if (*m_ppServerVM != nullptr)
    {
        return (*m_ppServerVM)->sqvm;
    }

    return nullptr;
}

template<ExecutionContext context>
void SquirrelManager::CompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column)
{
    m_logger->error("{} SCRIPT COMPILE ERROR", Util::GetContextName(context));
    m_logger->error("{} line = ({}) column = ({}) error = {}", sSource, line, column, sErr);
}

void SquirrelManager::RunServerCommand(const CCommand& args)
{
    ExecuteCode<CONTEXT_SERVER>(args.ArgS());
}

void SquirrelManager::RunClientCommand(const CCommand& args)
{
    ExecuteCode<CONTEXT_CLIENT>(args.ArgS());
}

template<ExecutionContext context>
void SquirrelManager::ExecuteCode(const char* code)
{
    HSQUIRRELVM v = nullptr;
    if (context == CONTEXT_SERVER)
    {
        v = GetServerSQVM();
    }
    else if (context == CONTEXT_CLIENT)
    {
        v = GetClientSQVM();
    }

    if (v != nullptr)
    {
        std::string strCode(code);
        m_logger->info("Executing {} code: {}", Util::GetContextName(context), strCode);
        CompileBufferState s(strCode);
        SQRESULT compileRes = sq_compilebuffer.Call<context>(v, &s, "console", -1, 1);
        m_logger->debug("sq_compilebuffer returned {}", compileRes);
        if (SQ_SUCCEEDED(compileRes))
        {
            sq_pushroottable.Call<context>(v);
            SQRESULT callRes = sq_call.Call<context>(v, 1, SQFalse, SQFalse);
            m_logger->debug("sq_call returned {}", callRes);
        }
    }
    else
    {
        m_logger->error("Cannot execute {} code, no handle to Squirrel VM for context", Util::GetContextName(context));
    }
}

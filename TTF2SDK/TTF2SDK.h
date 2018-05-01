#pragma once

typedef long SQInteger;

// TODO: Combine these
struct ClientVM
{
    unsigned char Data[8];
    HSQUIRRELVM sqvm;
};

// TODO: Combine these
struct ServerVM
{
    unsigned char Data[8];
    HSQUIRRELVM sqvm;
};

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

class TTF2SDK
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    ClientVM** m_ppClientVM = nullptr;
    ServerVM** m_ppServerVM = nullptr;
    FileSystemManager m_fsManager;
    
    std::mutex m_mutex;
    std::atomic_bool m_shouldRunClientCode = false;
    std::string m_clientCodeToRun = "";
    std::atomic_bool m_shouldRunServerCode = false;
    std::string m_serverCodeToRun = "";

    SourceInterface<IVEngineServer> m_engineServer;

public:
    ID3D11Device** m_ppD3D11Device = nullptr;

    TTF2SDK();
    ~TTF2SDK();

    FileSystemManager& GetFSManager();

    void RunFrameHook(double absTime, float frameTime);


    template<ExecutionContext context>
    SQInteger BasePrintHook(HSQUIRRELVM v);
    void PrintFunc(HSQUIRRELVM v, const SQChar* source, const SQChar* s, va_list args);

    template<ExecutionContext context>
    void CompilerErrorHook(HSQUIRRELVM v, const SQChar* sErr, const SQChar* sSource, SQInteger line, SQInteger column);

    void compileShaders();

    HSQUIRRELVM GetClientSQVM()
    {
        if (*m_ppClientVM != nullptr)
        {
            return (*m_ppClientVM)->sqvm;
        }

        return nullptr;
    }

    HSQUIRRELVM GetServerSQVM()
    {
        if (*m_ppServerVM != nullptr)
        {
            return (*m_ppServerVM)->sqvm;
        }

        return nullptr;
    }

    void SetClientCode(const std::string& code)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_clientCodeToRun = code;
        m_shouldRunClientCode = true;
    }

    std::string GetClientCode()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_shouldRunClientCode = false;
        return m_clientCodeToRun;
    }

    void SetServerCode(const std::string& code)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_serverCodeToRun = code;
        m_shouldRunServerCode = true;
    }

    std::string GetServerCode()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_shouldRunServerCode = false;
        return m_serverCodeToRun;
    }
};

TTF2SDK& SDK();
bool SetupSDK();

#pragma once

class BaseSigScanFunc
{
public:
    virtual void SetFuncPtr(void* funcPtr) = 0;
};

class SigScanFuncRegistry
{
private:
    struct RegistrationData
    {
        BaseSigScanFunc* func;
        const char* moduleName;
        const char* signature;
        const char* mask;
    };

    static const int MAX_SIG_SCAN_REGISTRATIONS = 100;

    RegistrationData m_registrations[MAX_SIG_SCAN_REGISTRATIONS];
    int m_numRegistrations;

public:
    static SigScanFuncRegistry& GetInstance();
    void AddSigScanFunc(BaseSigScanFunc& func, const char* moduleName, const char* signature, const char* mask);
    void ResolveAll();
};

template<typename T, typename... Args>
class SigScanFunc : public BaseSigScanFunc
{
protected:
    T(*m_func)(Args...) = nullptr;

public:
    SigScanFunc(const char* moduleName, const char* signature, const char* mask)
    {
        SigScanFuncRegistry::GetInstance().AddSigScanFunc(*this, moduleName, signature, mask);
    }

    void SetFuncPtr(void* ptr)
    {
        m_func = reinterpret_cast<T(*)(Args...)>(ptr);
    }

    void* GetFuncPtr()
    {
        return reinterpret_cast<void*>(m_func);
    }

    /*T operator()(Args&&... args)
    {
        return m_func(std::forward<Args>(args)...);
    }*/

    T operator()(Args... args)
    {
        return m_func(args...);
    }
};

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

template<typename T, typename... Args>
class HookedFunc : public SigScanFunc<T, Args...>
{
private:
    bool m_hooked = false;
    T(*m_hookedFunc)(Args...) = nullptr;

public:
    HookedFunc(const char* moduleName, const char* signature, const char* mask) : SigScanFunc(moduleName, signature, mask)
    {

    }

    void Hook(T(*detourFunc)(Args...))
    {
        if (m_hooked)
        {
            return;
        }

        auto logger = spdlog::get("logger");

        m_hookedFunc = m_func;

        MH_STATUS status = MH_CreateHookEx(m_hookedFunc, detourFunc, &m_func);
        if (status != MH_OK)
        {
            logger->critical("MH_CreateHook returned {}", status);
            throw std::exception("Failed to hook function");
        }

        status = MH_EnableHook(m_hookedFunc);
        if (status != MH_OK)
        {
            logger->critical("MH_EnableHook returned {}", status);
            throw std::exception("Failed to enable hook");
        }

        m_hooked = true;
        logger->debug("Hooked function at {} - trampoline location: {}", (void*)m_hookedFunc, (void*)m_func);
    }

    ~HookedFunc()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
        }
    }
};

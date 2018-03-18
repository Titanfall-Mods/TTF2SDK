#include "stdafx.h"

SigScanFuncRegistry& SigScanFuncRegistry::GetInstance()
{
    static SigScanFuncRegistry instance;
    return instance;
}

void SigScanFuncRegistry::AddSigScanFunc(BaseSigScanFunc& func, const char* moduleName, const char* signature, const char* mask)
{
    m_registrations[m_numRegistrations++] = {
        &func,
        moduleName,
        signature,
        mask
    };
}

void SigScanFuncRegistry::ResolveAll()
{
    auto logger = spdlog::get("logger");
    std::map<std::string, ModuleScan> moduleScanners;

    for (int i = 0; i < m_numRegistrations; i++)
    {
        RegistrationData& reg = m_registrations[i];
        std::string moduleName(reg.moduleName);

        // Check if the map contains a modulescan for the module
        // If not, try get a handle to the module with GetModuleHandle 
        // If no module handle available, resort to fancy loading callback stuff (TODO)
        if (moduleScanners.count(moduleName) == 0)
        {
            logger->debug("No ModuleScan found for {}", moduleName);

            // Create modulescan
            HMODULE module = GetModuleHandle(Util::Widen(moduleName).c_str());
            if (module != NULL)
            {
                logger->debug("Module {} already loaded, creating ModuleScan", moduleName);
                moduleScanners.emplace(moduleName, module);
            }
            else
            {
                throw std::exception("TODO: Wait for module to get loaded");
            }
        }

        ModuleScan& moduleScan = moduleScanners.at(moduleName);
        void* ptr = moduleScan.Scan(reg.signature, reg.mask);
        logger->debug("Signature {} in {} found at {}", Util::DataToHex(reg.signature, strlen(reg.mask)), moduleName, ptr);
        reg.func->SetFuncPtr(ptr);
    }
}

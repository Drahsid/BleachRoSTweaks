#include "ConsoleManager.hpp"
#include "Memory.hpp"
#include "safetyhook.hpp"

static HMODULE sEngineModule = nullptr;

typedef void (*pfn_SPFXEngine_Update)(void);
static pfn_SPFXEngine_Update sSPFXEngine_UpdateFn = (pfn_SPFXEngine_Update)nullptr;

static SafetyHookInline sSPFXEngine_UpdateHook;
void SPFXEngine_Update(void) {
    //printf("SPFXEngine_Update called!\n");
    sSPFXEngine_UpdateHook.call();
}

void InitHooks(void) {
    sSPFXEngine_UpdateFn = reinterpret_cast<pfn_SPFXEngine_Update>(GetProcAddress(sEngineModule, "SPFXEngine_Update"));
    if (sSPFXEngine_UpdateFn != nullptr) {
        sSPFXEngine_UpdateHook = safetyhook::create_inline(sSPFXEngine_UpdateFn, SPFXEngine_Update);
    }
    else {
        printf("SPFXEngine_EndDraw not found!\n");
    }
}

void Init(void) {
    ConsoleManager::Allocate();

    sEngineModule = GetModuleHandleA("SPFXEngine.dll");
    if (sEngineModule == nullptr) {
        printf("Failed to get engine module!\n");
    }

    InitHooks();

    printf("ASI Plugin initialization complete.\n");
}

void Shutdown(void) {
    ConsoleManager::Free();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hinstDLL);
            Init();
            break;
        }
        case DLL_PROCESS_DETACH: {
            Shutdown();
            break;
        }
        default:
            break;
    }
    return TRUE;
}

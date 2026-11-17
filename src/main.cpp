#include <stddef.h>
#include <stdint.h>

#include "main.h"

#include "utils.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"

#include "patches/registry.h"

void* operator new(size_t size)
{
    return operator_new(size);
}

void operator delete(void* p)
{
    return operator_delete(p);
}

static void Patch_MainAfterLoadAscendCfg(P_WinMgr pWinMgr)
{
    WinMgr_LoadAscendCfg(pWinMgr);
    Registry::CallHook(1);
}

/*
static void __cdecl PrintDebugInfoOnStart(const char* title)
{
    printf(title);
    printf("cseg01: %X\n", cseg01_start);
    printf("dseg02: %X\n", dseg02_start);
    printf("cseg03: %X\n", cseg03_start);
    printf("dseg04: %X\n", dseg04_start);
}
*/

void PatchInitRtns(unsigned int limit)
{
    // Exclude all technical CRT initializations from our list;
    // we don't need them, and they could only corrupt the state of the original environment
    rt_init* current = &__Start_XI;
    rt_init* end = &__End_XI;
    while (current < end)
    {
        if (current->priority < 0x40)
        {
            current->rtn_type = 2;
        }
        current++;
    }
    // Call our static initialization, where patches should be created and registered
    __LocalInitRtns(limit);
    // Now we can call the patches for the first time
    Registry::CallHook(0);
}

static void __declspec(naked) PatchInitRtns_()
{
    _asm
    {
        push    eax
        call    PatchInitRtns
                // Continue the original initialization
                // EBP must not be changed, it is used in __sys_init_387_emulator
                // Therefore, __InitRtns cannot be called inside the standard prologue/epilogue
        pop     eax
        call  __InitRtns
        ret
    }
}

/*
  C Runtime is not initialized here
  MEMORY PATCHING ONLY ALLOWED
*/
void attach(void)
{
    // The moment __InitRtns is called in the startup code
    WriteMemory(0x00078637 + 1, PatchInitRtns_, false);
    // The moment LoadAscendCfg is loaded - all windows from windows.txt are already constructed
    WriteMemory(0x0001CB47 + 1, Patch_MainAfterLoadAscendCfg, false);
}

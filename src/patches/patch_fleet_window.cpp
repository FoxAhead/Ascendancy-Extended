#include "patches/patch_fleet_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchFleetWindow)

void __stdcall Patch_FleetWndDrawListItemFunc1(char* s, P_Planet pPlanet)
{
    char* p = s + strlen(s);
    int days = Planet_GetDaysToCompleteItem(pPlanet);
    if (days == 0xFFFF)
    {
        sprintf(p, "\n%s", StaticTxtRead(138));  // 138: " - No Progress"
    }
    else
    {
        p += sprintf(p, "\n");
        sprintf(p, StaticTxtRead(139), days, StaticTxtRead((days == 1) ? 0x1C : 0x1D));  // 139: " - %d day%s until completion"
    }
}

static void __declspec(naked) Patch_FleetWndDrawListItemFunc1_()
{
    _asm
    {
        pushad
        push    esi  // pPlanet
        lea     eax, [esp+190h-168h]  // s
        push    eax
        call    Patch_FleetWndDrawListItemFunc1
        popad
        jmp     Cache_GetShapeTable
    }
}

void PatchFleetWindow::Hook(int stage)
{
    // Fleet Window (ships list)
    switch (stage)
    {
        case 0:
            WriteMemory(0x000510FE + 1, Patch_FleetWndDrawListItemFunc1_, false);
            WriteMemory(0x00051162 + 1, Patch_FleetWndDrawListItemFunc1_, false);
            break;
    }
}

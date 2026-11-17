#include "patches/patch_battle_mode_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchBattleModeWindow)

static void TargetWorldPos(P_Target t, T_Vector3D* out)
{
    switch (t->type)
    {
        case 0:
            out->x = out->y = out->z = 0.0;
            break; // Star — system center
        case 1:
            *out = t->pObject.pPlanet->position;
            break;
        case 2:
            *out = t->pObject.pShip->position;
            break;
        case 3:
        {
            P_Lane l = t->pObject.pLane;
            *out = (Game.pCurStar == l->pStar1) ? l->vector1 : l->vector2;
        }
        break;
        default:
            out->x = out->y = out->z = 0.0;
            break;
    }
}

void __stdcall Patch_Wnd04BM_Proc4_PrintLabel(P_Wnd04BM pWnd, char* s)
{
    if (pWnd->pSelectedBatDisplayItem && pWnd->pHoveredBatDisplayItem && pWnd->pSelectedBatDisplayItem != pWnd->pHoveredBatDisplayItem)
    {
        T_Vector3D a, b;
        TargetWorldPos(&pWnd->pSelectedBatDisplayItem->target, &a);
        TargetWorldPos(&pWnd->pHoveredBatDisplayItem->target, &b);
        float d = VectorsDistance(&a, &b) / 32.0;
        char* p = s + strlen(s);
        sprintf(p, " d: %.3g", d);
    }
}

static void __declspec(naked) Patch_Wnd04BM_Proc4_PrintLabel_()
{
    _asm
    {
        pushad
        push    ecx  // s
        push    ebp  // pWnd
        call    Patch_Wnd04BM_Proc4_PrintLabel
        popad
        jmp     Font_DrawFormattedText
    }
}

void PatchBattleModeWindow::Hook(int stage)
{
    // Battle Mode Window (System Display)
    switch (stage)
    {
        case 0:
            WriteMemory(0x00018E33 + 1, Patch_Wnd04BM_Proc4_PrintLabel_, false);
            break;
            /*
            case 1:
                P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "BatModeWnd", 0);
                if (pWnd)
                {
                    procs = Wnd04BM_Procs;
                    procs.proc3_msg = (P_Proc3)Patch_Wnd04BM_Proc3;
                    // procs.proc4_draw = (P_Proc4)Patch_Wnd04BM_Proc4;
                    pWnd->pProcs = &procs;
                }
                break;
                */
    }
}

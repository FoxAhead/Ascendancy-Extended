#include "patches/patch_debug_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchDebugWindow)

void Patch_BuildWListBox(P_Wnd13DBGWND pDbgWnd, P_Wnd10LB pListBox)
{
    int currentState;
    int i;      
    WORD windowCount;
    int hwnd;        
    T_WndA2* pWndA2; 
    char v8;         
    char s[72];      
    char sc[20];      

    currentState = WinMgr.currentState;
    pDbgWnd->tabIndex = 0;
    i = 31;
    windowCount = WinMgr.winStates[currentState]._windowCount;
    while (windowCount > 0)
    {
        // if (i < 0)
        // {
            // Q_AssertLogBreakExit_sub_26198(0, "..\\winmgr.cpp", 0x131E);
        // }
        hwnd = WinMgr.winStates[WinMgr.currentState].hwnds[i];
        if (hwnd)
        {
            pWndA2 = WinMgr.wnds[hwnd].pWndA2;
            if (pWndA2->parentHwnd)
            {
                sprintf(sc, "C%d", pWndA2->parentHwnd);
            }
            else
            {
                strcpy(sc, "    ");
            }
            sprintf(s, "%d: %s HWND %03d %s", i, sc, hwnd, pWndA2->name);
            --windowCount;
            Wnd10LB_AddItem(pListBox, s, 0xFFFF, 0);
        }
        --i;
    }
}

void PatchDebugWindow::Hook(int stage)
{
    // Cosmos Window
    switch (stage)
    {
        case 0:
            WriteMemory(0x0005AE68, Patch_BuildWListBox, false, OP_JMP);
            break;

            // case 1:
            // procs = Wnd02COSW_Procs;
            // procs.proc4_draw = (P_Proc4)Wnd02COSWx_Proc4;
            // P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "COSMOSWnd", 0);
            // if (pWnd)
            // {
            // pWnd->pProcs = &procs;
            // }

            // break;
    }
}

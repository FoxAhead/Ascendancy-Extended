#include "patches/patch_nextturn.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "utils.h"

REGISTER_PATCH(PatchNextTurn)

static int prevEnterTick = 0;

static int Patch_WndNextTurn_Proc3(P_WndA2 pWnd, UWORD message, int param1, int param2)
{
    if (message == 3 && param2 == KSC_RETURN)
    {
        if ((unsigned)(TickCounter - prevEnterTick) < 50)
            return TRUE;
        prevEnterTick = TickCounter;
        return WndA2_Procs.proc3_msg(pWnd, 4, pWnd->pane.x0, pWnd->pane.y0);
    }
    return WndA2_Procs.proc3_msg(pWnd, message, param1, param2);
}

void PatchNextTurn::Hook(int stage)
{
    switch (stage)
    {
        case 1:
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "NEXTTURN", NULL);
            if (pWnd)
            {
                procs = WndA2_Procs;
                procs.proc3_msg = Patch_WndNextTurn_Proc3;
                pWnd->pProcs = &procs;
            }
            break;
    }
}

#include "patches/patch_research.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchResearch)

static int prevEnterTick = 0;

static int Patch_WndResearch_Proc3(P_Wnd09RW pWnd, UWORD message, int param1, int param2)
{
    switch (message)
    {
        case 3:
            switch (param2)
            {
                case KSC_NUMPAD9:
                case KSC_NUMPAD3:
                    pWnd->camera.position.y += (param2 == KSC_NUMPAD3) ? 10 : -10;
                    pWnd->super.pProcs->proc4_draw(&pWnd->super, 0);
                    Wnd09RW_HoverTest(pWnd);
                    return 0;
            }
    }
    return Wnd09RW_Procs.proc3_msg((P_WndA2)pWnd, message, param1, param2);
}

// proc4: original tree + number of days in the center of each available circle
static void Patch_WndResearch_Proc4(P_Wnd09RW pWnd, int a2)
{
    char s[12];
    int i, rp, done, days;
    UWORD cur;

    Wnd09RW_Procs.proc4_draw((P_WndA2)pWnd, a2);

    rp = Research_GetTotalRaceResearch(&Research, PlayerRaceIndex);  // research points per day
    cur = Research.current.project[PlayerRaceIndex];

    for (i = 0; i < Research.num; ++i)
    {
        // only those not yet researched by the player and available for research
        if (((1 << PlayerRaceIndex) & Research.techs[i].knownToRace))
            continue;
        if (!Research_TechIsAvailableToRace(&Research, (UWORD)i, PlayerRaceIndex))
            continue;

        // for the current project, we take into account the accumulated progress, for the rest - from zero
        // done = (cur == i) ? Research.current.progress[PlayerRaceIndex] : 0;
        done = Research.current.progress[PlayerRaceIndex];
        days = GetTurnsToComplete(Research.techs[i].cost, done, rp);
        if ((unsigned)days == 0xFFFF)  // rp == 0 - no laboratories
            continue;

        sprintf(s, "%d", days);
        WinMgr.SmallFont.pane = pWnd->super.pane;
        // flags 0xC = center by X and Y relative to the point; wipeColor 0xFF — without wiping
        Font_DrawFormattedText(
            &WinMgr.SmallFont,
            pWnd->diskX[i],
            pWnd->diskY[i],
            s,
            0xC,
            (cur == i) ? 4 * Game.races[PlayerRaceIndex].colorSet + 0x13 : 0xF3,
            0xFF,
            0);
    }
}

void PatchResearch::Hook(int stage)
{
    switch (stage)
    {
        case 1:
            // Research
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "RESWIN", 0);
            if (pWnd)
            {
                procs = Wnd09RW_Procs;
                procs.proc3_msg = (P_Proc3)Patch_WndResearch_Proc3;
                procs.proc4_draw = (P_Proc4)Patch_WndResearch_Proc4;
                pWnd->pProcs = &procs;
            }
            break;
    }
}

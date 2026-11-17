#include "patches/patch_mainbtns.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchMainBtns)

static void Patch_WndMainBtns_Proc4(P_WndA2 pWnd, int a2)
{
    // Original draw
    WndA2_Procs.proc4_draw(pWnd, a2);

    // Additional draw
    char s[60] = "";
    P_Race pRace = &Game.races[PlayerRaceIndex];
    bool warning = false;

    if (IsWndName(pWnd, "STATUS"))
    {
        int nPlanets = 0;
        for (int i = 0; i < Game.planetsNum; ++i)
        {
            P_Planet pPlanet = &Game.planets[i];
            if (pPlanet->raceIndex == PlayerRaceIndex)
            {
                ++nPlanets;
                if (pPlanet->buildingPlanetItemIndex == 0xFF && pPlanet->population > pPlanet->usedPopulation)
                {
                    warning = true;
                }
            }
        }
        int nSystems = 0;
        UBYTE raceMask = 1 << PlayerRaceIndex;
        for (int i = 0; i < Game.starsNum; ++i)
        {
            P_Star pStar = &Game.stars[i];
            if (pStar->planetsRacesBits == raceMask && pStar != pRace->homeStar)
                ++nSystems;
        }
        if ((raceMask & pRace->homeStar->planetsRacesBits) != 0)
            ++nSystems;
        sprintf(s, "%d Planets, %d Systems", nPlanets, nSystems);
    }
    else if (IsWndName(pWnd, "FLEET")) /* Ships */
    {
        int nShips = Race_GetShips(pRace, 0, 0);
        int nMore = Race_GetMoreAllowedShips(pRace);
        sprintf(s, "%d of %d", nShips, nShips + nMore);
    }
    else if (IsWndName(pWnd, "RESEARCH"))
    {
        int rp = Research_GetTotalRaceResearch(&Research, PlayerRaceIndex);
        if (rp)
        {
            UWORD proj = Research.current.project[PlayerRaceIndex];
            if (proj != 0xFFFF)
            {
                int days = GetTurnsToComplete(Research.techs[proj].cost, Research.current.progress[PlayerRaceIndex], rp);
                if (days == 1)
                    sprintf(s, "%d day", days);
                else
                    sprintf(s, "%d days", days);
            }
            else
            {
                // V_AllTechsKnownCache = 0;
                // WriteMemory(0x0010509C, NULL, false, 0, 0, 0, 0);
                if (!Race_HasDiscoveredAllTechs(&Game.races[PlayerRaceIndex]))
                {
                    warning = true;
                }
            }
            // if(a2){
              // char s2[10];
              // sprintf(s2, "%d", rp);
              // WinMgr.SmallFont.pane = pWnd->pane;
              // Font_DrawFormattedText(&WinMgr.SmallFont, 95, 16, s2, ASCEND_TF_HALIGN_RIGHT_X, 0xFFFF, 0xFF, 0);
            // }
        }
    }
    else if (IsWndName(pWnd, "SPECIALAB"))
    {
        int need = AbilityDays[pRace->ability];
        if (need)
        {
            int willTakeDays = need - pRace->daysAfterAbilityUse;
            if (willTakeDays < 1)
                strcpy(s, "Ready");
            else if (willTakeDays == 1)
                sprintf(s, "%d day", willTakeDays);
            else
                sprintf(s, "%d days", willTakeDays);
        }
    }
    if (s[0])
    {
        WinMgr.SmallFont.pane = pWnd->pane;
        Font_DrawFormattedText(&WinMgr.SmallFont, 2, 16, s, 0, 0xFFFF, 0xFF, 0);
    }
    if (warning)
    {
        strcpy(s, "(!)");
        WinMgr.SmallFont.pane = pWnd->pane;
        Font_DrawFormattedText(&WinMgr.SmallFont, 95, 4, s, 0, 27, 0xFF, 0);
    }
}

static int Patch_WndMainBtns_Proc3(P_WndA2 pWnd, UWORD message, int param1, int param2)
{
    int result = WndA2_Procs.proc3_msg(pWnd, message, param1, param2);
    if (message == 0xD || message == 0xE)
    {
        Patch_WndMainBtns_Proc4(pWnd, 0);
    }
    return result;
}

void PatchMainBtns::Hook(int stage)
{
    switch (stage)
    {
        case 1:

            static const char* names[4] = {"STATUS", "FLEET", "RESEARCH", "SPECIALAB"};
            procs = WndA2_Procs;
            procs.proc3_msg = Patch_WndMainBtns_Proc3;
            procs.proc4_draw = Patch_WndMainBtns_Proc4;
            for (int i = 0; i < 4; ++i)
            {
                P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, (char*)names[i], NULL);
                if (pWnd)
                {
                    pWnd->pProcs = &procs;
                }
            }
            break;
    }
}

#include "patches/patch_cosmos_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchCosmosWindow)

static int Patch_Wnd02COSW_Proc3(P_Wnd02COSW pWnd, UWORD message, int param1, int param2)
{
    switch (message)
    {
        case ASCEND_WM_KEYBOARD:
            switch (param2)
            {
                case KSC_R:
                    if (CheatMode == TRUE && (Keyboard.Shift))
                    {
                        for (int i = 0; i < Game.racesNum; ++i)
                        {
                            if (Game.races[i].treaties[PlayerRaceIndex])
                            {
                                Game.races[i].attitudes[PlayerRaceIndex] = 450;
                            }
                        }
                        pWnd->super.pProcs->proc4_draw((P_WndA2)pWnd, 0);
                        return 0;
                    }
                    break;
            }
            return Wnd02COSW_Procs.proc3_msg((P_WndA2)pWnd, message, param1, param2);
            break;

        case ASCEND_WM_RIGHTCLICK:
            if (WinMgr.currentState == ASCEND_WINMGR_STATE_MAIN_SCREEN)
            {
                if (InsidePane(&pWnd->super.pane, param1, param2) && pWnd->pHoveredDisplayItem && !WinMgr.autoTurn)
                {
                    switch (pWnd->pHoveredDisplayItem->targetType)
                    {
                        case ASCEND_TARGET_TYPE_0_STAR:
                        {
                            P_Star pClickedStar = pWnd->pHoveredDisplayItem->pObject.pStar;
                            if ((PlayerRaceMask & pClickedStar->exploredBits) != 0)
                            {
                                Game.pCurStar = pClickedStar;
                                pWnd->super.pProcs->proc4_draw((P_WndA2)pWnd, 0);
                            }
                            break;
                        }
                        case ASCEND_TARGET_TYPE_2_SHIP:
                        {
                            P_Ship pClickedShip = pWnd->pHoveredDisplayItem->pObject.pShip;
                            if (pClickedShip->raceIndex == PlayerRaceIndex)
                            {
                                P_Wnd08SW pWnd08SW = (P_Wnd08SW)WinMgr_FindWnd(&WinMgr, "SHIPDESSCREEN", 0);
                                Wnd08SW_Open(pWnd08SW, pClickedShip, 0, TRUE);
                                WinMgr_QueueMessage(&WinMgr, ASCEND_SM_SWITCHSTATE, ASCEND_WINMGR_STATE_SHIP_DESIGN, 1);
                                break;
                            }
                        }
                    }
                }
                return TRUE;
            }
            break;
    }

    return Wnd02COSW_Procs.proc3_msg((P_WndA2)pWnd, message, param1, param2);
}

static void Patch_Wnd02COSW_Proc4(P_Wnd02COSW pWnd, int a2)
{
    Wnd02COSW_Procs.proc4_draw((P_WndA2)pWnd, a2);
    if (CheatMode == TRUE && pWnd->showGalaxyDebugInfo == TRUE)
    {
        char s[10];
        WinMgr.LargeFont.pane = GSystem.pane;
        int y = 63;
        for (int i = 0; i < Game.racesNum; ++i)
        {
            if (Game.races[i].treaties[PlayerRaceIndex])
            {
                int color = 4 * Game.races[i].colorSet + 0x13;
                sprintf(s, "%d", Game.races[i].attitudes[PlayerRaceIndex]);
                Font_DrawFormattedText(&WinMgr.LargeFont, 137, y, s, 0, color, 0xFF, 0);
            }
            y += 13;
        }
    }
}

void __cdecl Patch_UpdateHover(P_Wnd02COSW pWnd)
{
    char s[100];
    if (pWnd->pHoveredDisplayItem->targetType == ASCEND_TARGET_TYPE_2_SHIP)
    {
        P_Ship pShip = pWnd->pHoveredDisplayItem->pObject.pShip;
        if (pShip->raceIndex == PlayerRaceIndex)
        {
            char* p = s;
            p += sprintf(s, "%s", pShip->name);

            switch (pShip->locationType)
            {
                case ASCEND_LOCATION_TYPE_SHIPDOCK:
                {
                    p = stpcpy(p, "\n");
                    p += sprintf(p, StaticTxtRead(0x94), pShip->pLocation.pPlanet->name);  // 148: "Being Refitted at %s"
                    int days = Planet_GetDaysToCompleteItem(pShip->pLocation.pPlanet);
                    if (days != 0xFFFF)
                    {
                        p = stpcpy(p, "\n");
                        sprintf(p, StaticTxtRead(139), days, StaticTxtRead((days == 1) ? 0x1C : 0x1D));  // 139: " - %d day%s until completion"
                    }
                    break;
                }
                case ASCEND_LOCATION_TYPE_ORBIT:
                    p = stpcpy(p, "\n");
                    sprintf(p, StaticTxtRead(0x8F), pShip->pLocation.pPlanet->name);  // 143: "Orbiting %s"
                    break;
                case ASCEND_LOCATION_TYPE_STARLANE:
                    P_Lane pLane = pShip->pLocation.pLane;
                    float travelSpeed = pShip->totalStarLaneHyperdrivePotential + pShip->totalStarLaneDrivePotential;
                    if ((pLane->flags & ASCEND_LANE_FLAGS_1_RED_LINK) != 0)
                    {
                        travelSpeed = travelSpeed * 0.33333334;
                    }
                    if (Game.races[pShip->raceIndex].ability == ASCEND_SA_FAST_LANE_TRAVEL)
                    {
                        if (travelSpeed < 1.0)
                        {
                            travelSpeed = 1.0;
                        }
                        travelSpeed = travelSpeed * 2.0;
                    }
                    int etaDays = (int)((LaneDistance(pLane) - pShip->position.x) / travelSpeed) + 1;
                    P_Star destStar = (pShip->position.y) ? pLane->pStar2 : pLane->pStar1;
                    char* name = (((1 << PlayerRaceIndex) & destStar->exploredBits) != 0) ? destStar->name : StaticTxtRead(0x91);
                    // 146: "In Starlane to %s System\nETA - %d Day%s"
                    p = stpcpy(p, "\n");
                    sprintf(p, StaticTxtRead(0x92), name, etaDays, StaticTxtRead((etaDays > 1) ? 0x1D : 0x1C));
                    break;
            }

            int y = pWnd->super.pane.y1 - 27;
            int x = pWnd->super.pane.x1 - 130;
            Font_DrawFormattedText(&WinMgr.SmallFont, x, y, s, ASCEND_TF_HALIGN_CENTER_X, 0xFFFF, 0xFF, 0);
        }
    }
}

static void __declspec(naked) Patch_UpdateHover_()
{
    _asm
    {
        push    ebp  // pWnd
        call    Patch_UpdateHover
        add     esp, 04h
        add     esp, 1Ch
        pop     ebp
        pop     edi
        pop     esi
        pop     ecx
        ret
    }
}

void PatchCosmosWindow::Hook(int stage)
{
    // Cosmos Window
    switch (stage)
    {
        case 0:
            WriteMemory(0x00024AC3 + 2, Patch_UpdateHover_, false);
            WriteMemory(0x000249E5 + 1, NULL, false, 0xFF);
            break;

        case 1:
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "COSMOSWnd", 0);
            if (pWnd)
            {
                procs = Wnd02COSW_Procs;
                procs.proc3_msg = (P_Proc3)Patch_Wnd02COSW_Proc3;
                procs.proc4_draw = (P_Proc4)Patch_Wnd02COSW_Proc4;
                pWnd->pProcs = &procs;
            }
            break;
    }
}

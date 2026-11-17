#include "patches/patch_planet.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchPlanet)

// Square outline colors - the same ones the original uses for the grid (switch on color&7)
static const int OutlineCol[5] = {0x00, 0x0C, 0x75, 0x92, 0xFF};
// Haze tint tables for optional variant 1: BLACK04, WHITE00, RED01, GREEN01, BLUE01
static const int TintHaze[5] = {0x02, 0x0B, 0x08, 0x09, 0x0A};
static const int TintHaze2[5] = {7, 14, 8, 9, 10};
// static.txt line indices: 75 "White", 76 "Black", 77 "Red", 79 "Green", 81 "Blue"
static const int ColorText[5] = {76, 75, 77, 79, 81};

static void DrawEdges(PANE* pPane, SCRNVERTEX* vlist, unsigned char color)
{
    for (int i = 0; i < 4; ++i)
    {
        BYTE idxFrom = LineIndexFromTo[0][i];
        BYTE idxTo = LineIndexFromTo[1][i];
        VFX_line_draw(pPane, vlist[idxFrom].x, vlist[idxFrom].y, vlist[idxTo].x, vlist[idxTo].y, LD_DRAW, color);
    }
}

static POINT GetSurfaceSquareDiamondTop(int row, int column)
{
    POINT result = {410 + 34 * row - 34 * column, 110 + 17 * row + 17 * column};
    return result;
}

static POINT GetOrbitalSquareTopLeft(int orbitalIndex)
{
    POINT result = {13 + 70 * (orbitalIndex / 5), 119 + 70 * (orbitalIndex % 5)};
    return result;
}

static POINT GetPlanetSquareHotPoint(P_Planet pPlanet, int squareIndex)
{
    POINT result;
    if (squareIndex < pPlanet->surfaceSquaresNum)
    {
        T_SquareGridCoord coords = Planet_GetSurfaceSquareGridCoords(pPlanet, squareIndex);
        result = GetSurfaceSquareDiamondTop(coords.row, coords.column);
    }
    else
    {
        result = GetOrbitalSquareTopLeft(squareIndex - Game.pCurPlanet->surfaceSquaresNum);
        result.x += 35;
        result.y += 35;
    }
    return result;
}

static void SetFullDiamond(SCRNVERTEX* full, int topX, int topY)
{
    full[0].x = topX;
    full[1].x = full[0].x + 34;
    full[2].x = full[0].x;
    full[3].x = full[0].x - 34;
    full[0].y = topY;
    full[1].y = full[0].y + 17;
    full[2].y = full[0].y + 34;
    full[3].y = full[0].y + 17;
}

static void SetInsetDiamond(SCRNVERTEX* full, SCRNVERTEX* inset, bool buildable)
{
    int dx, dy;
    if (buildable)
    {
        dx = 2;
        dy = 1;
    }
    else
    {
        dx = 28;
        dy = 14;
    }
    inset[0].x = full[0].x;
    inset[0].y = full[0].y + dy;
    inset[1].x = full[1].x - dx;
    inset[1].y = full[1].y;
    inset[2].x = full[2].x;
    inset[2].y = full[2].y - dy;
    inset[3].x = full[3].x + dx;
    inset[3].y = full[3].y;
}

static int AnyBuildable(P_Planet pPlanet, UWORD sq)
{
    int item;
    for (item = 0; item < ASCEND_PI_39_END; ++item)
    {
        if (item == ASCEND_PI_05_COLONY_BASE) continue;
        if (Planet_CanPlaceItem(pPlanet, (UBYTE)item, sq) == TRUE)
            return 1;
    }
    return 0;
}

static int g_shiftCache = 0;

static int Patch_WndPlanet_Proc3(P_Wnd07PW pWnd, UWORD message, int param1, int param2)
{
    // FILE *fp = fopen("_DBGX.TXT", "at");
    // fprintf(fp, "message=%d param1=%d param2=%d\n", message, param1, param2);
    // fclose(fp);
    // flushall();

    switch (message)
    {
        case ASCEND_WM_ACTIVATE:
            WinMgr_AddRepeatingEvent(&WinMgr, pWnd->super.hwnd, 0x198, 0, 0, 2);
            break;
        case ASCEND_WM_DEACTIVATE:
            WinMgr_RemoveRepeatingEvent(&WinMgr, pWnd->super.hwnd, 0x198);
            break;
        case 0x198:
            if (g_shiftCache != Keyboard.Shift)
            {
                g_shiftCache = Keyboard.Shift;
                if (pWnd->super.shown)
                {
                    pWnd->super.pProcs->proc4_draw(&pWnd->super, 0);
                }
            }
            return TRUE;
        case ASCEND_WM_KEYBOARD:
            switch (param2)
            {
                case KSC_C:
                    pWnd->displayFlags ^= 2u;
                    pWnd->super.pProcs->proc4_draw(&pWnd->super, 0);
                    return 0;
                case KSC_P:
                    pWnd->displayFlags ^= 4u;
                    pWnd->super.pProcs->proc4_draw(&pWnd->super, 0);
                    return 0;
                case KSC_H:
                    pWnd->displayFlags ^= 8u;
                    pWnd->super.pProcs->proc4_draw(&pWnd->super, 0);
                    return 0;
                case KSC_SPACE:
                    gHazeNum++;
                    if (gHazeNum > 22) gHazeNum = 0;
                    break;
                case KSC_NUMPAD2:
                    gDY++;
                    break;
                case KSC_NUMPAD8:
                    gDY--;
                    break;
                case KSC_NUMPAD6:
                    gDX++;
                    break;
                case KSC_NUMPAD4:
                    gDX--;
                    break;
            }
            break;
        case ASCEND_WM_LEFTCLICK:
            int r = Wnd07PW_Procs.proc3_msg((P_WndA2)pWnd, message, param1, param2);
            P_Planet pPlanet = Game.pCurPlanet;
            UWORD sq;
            P_WndA2 pBut;

            if (pPlanet && pPlanet->raceIndex == PlayerRaceIndex)
            {
                sq = (UWORD)GetSquareIndexAtScreen(param1, param2);
                if (sq != 0xFFFF && pPlanet->pSquares[sq].planetItemIndex == 0xFF && AnyBuildable(pPlanet, sq))
                {
                    pBut = WinMgr_FindWnd(&WinMgr, "Build", 0);  // the first button = Build
                    if (pBut && pBut->shown == TRUE)
                    {
                        WndA2_Procs.proc3_msg(pBut, 4, (pBut->pane.x0 + pBut->pane.x1) / 2, (pBut->pane.y0 + pBut->pane.y1) / 2);
                    }
                }
            }
            return r;
            break;
    }
    return Wnd07PW_Procs.proc3_msg((P_WndA2)pWnd, message, param1, param2);
}

// Planet Window - draw function
static void Patch_WndPlanet_Proc4(P_Wnd07PW pWnd, int a2)
{
    // Original draw
    Wnd07PW_Procs.proc4_draw((P_WndA2)pWnd, a2);

    // Additions
    char s[96];
    P_Planet pPlanet = Game.pCurPlanet;

    if (pPlanet->raceIndex == PlayerRaceIndex && pWnd->hoveredSquareIndex != 0xFFFF)  // && pWnd->hoveredSquareIndex < pPlanet->surfaceSquaresNum)
    {
        T_Square* pSquare = &pPlanet->pSquares[pWnd->hoveredSquareIndex];
        if (pSquare->planetItemIndex != 0xFF)
        {
            P_PlanetItem pPlanetItem = &PlanetItems.item[pSquare->planetItemIndex];
            int squareColor = pSquare->color & 7;
            char* p = s;
            if (pSquare->flags & ASCEND_SQUARE_FLAGS_AUTOMATED)
            {
                p += sprintf(p, "%s\n", StaticTxtRead(84));  // 84: "Automated  "
            }
            p += sprintf(p, "%s", pPlanetItem->name);
            if (pSquare->planetItemIndex == ASCEND_PI_23_SHIP)
            {
                P_Ship pShip = Planet_GetShipAtSquare(pPlanet, pWnd->hoveredSquareIndex);
                p += sprintf(p, " \"%s\"", pShip->name);
            }
            if (Keyboard.Shift)
            {
                if (pPlanetItem->res)
                {
                    p += sprintf(p, "\n+ %d research", pPlanetItem->res + (pSquare->color == ASCEND_SQUARE_COLOR_BLUE));
                }
                if (pPlanetItem->ind)
                {
                    p += sprintf(p, "\n+ %d industry", pPlanetItem->ind + (pSquare->color == ASCEND_SQUARE_COLOR_RED));
                }
                if (pPlanetItem->pros)
                {
                    p += sprintf(p, "\n+ %d prosperity", pPlanetItem->pros + (pSquare->color == ASCEND_SQUARE_COLOR_GREEN));
                }
                switch (pSquare->planetItemIndex)
                {
                    case ASCEND_PI_14_INTERNET:
                        p = stpcpy(p, "\nx1.5 research");
                        break;
                    case ASCEND_PI_12_HYPERPOWER_PLANT:
                        p = stpcpy(p, "\nx1.5 industry");
                        break;
                    case ASCEND_PI_13_FERTILIZATION_PLANT:
                        p = stpcpy(p, "\nx1.5 prosperity");
                        break;
                }
            }

            POINT hotPoint = GetPlanetSquareHotPoint(pPlanet, pWnd->hoveredSquareIndex);
            WinMgr.SmallFont.pane = pWnd->super.pane;
            int tw = GetMultiLineTextWidth(&WinMgr.SmallFont, s);
            int th = GetMultiLineTextHeight(&WinMgr.SmallFont, s);
            RECT rect = {hotPoint.x - tw / 2, hotPoint.y - th, hotPoint.x + tw / 2, hotPoint.y};
            int padding = 2;
            InflateRect(&rect, 3 + padding, 1 + padding);
            OffsetRect(&rect, rect.x0 < 8 ? 8 - rect.x0 : 0, -18);
            SCRNVERTEX quad[4];
            RectToQuad(rect, quad);
            VFX_translate_polygon(&GSystem.pane, 4, quad, (*GSystem.pHazeTables)[7]);
            VFX_translate_polygon(&GSystem.pane, 4, quad, (*GSystem.pHazeTables)[TintHaze2[squareColor]]);
            DrawEdges(&GSystem.pane, quad, 0xF);
            POINT centerPoint = RectCenterPoint(rect);
            DrawTextShaded(&WinMgr.SmallFont, centerPoint.x, centerPoint.y + 1, s, ASCEND_TF_HALIGN_CENTER_X | ASCEND_TF_VALIGN_CENTER_Y, 0xFFFF);
        }
    }
    // Planet name
    int color = 4 * Game.races[pPlanet->raceIndex].colorSet + 0x13;
    WinMgr.LargeFont.pane = pWnd->super.pane;
    Font_DrawFormattedText(&WinMgr.LargeFont, 0, 450, pPlanet->name, ASCEND_TF_HALIGN_CENTER_PANE, color, 0xFF, 0);
    if (pPlanet->isSelfManaged)
    {
        WinMgr.SmallFont.pane = pWnd->super.pane;
        Font_DrawFormattedText(&WinMgr.SmallFont, 0, 463, StaticTxtRead(0x81), ASCEND_TF_HALIGN_CENTER_PANE, color, 0xFF, 0);
    }

    // Debug info
    // sprintf(s, "%d", pWnd->hoveredSquareIndex);
    // Font_DrawFormattedText(&WinMgr.LargeFont, 100, 100, s, 0, 0xFFFF, 0xFF, 0);
}

void __cdecl Patch_DrawSurfaceItem(P_Wnd07PW pWnd, P_Square pSquare, int savedRet, PANE* pane, void* shape_table, LONG shape_number, LONG hotX, LONG hotY)
{
    // int flag2 = pWnd->displayFlags & 2;
    bool extraInfo = Keyboard.Shift or (pWnd->displayFlags & 2);

    if (!(pWnd->displayFlags & 8) || Game.pCurPlanet->raceIndex != PlayerRaceIndex)
    {
        if (Game.pCurPlanet->raceIndex == PlayerRaceIndex && (pSquare->color == 0 || pSquare->color >= 2 && pSquare->color <= 4) && extraInfo)
        {
            // Draw tinted in square color
            VFX_shape_lookaside((*GSystem.pHazeTables)[TintHaze2[pSquare->color]]);
            VFX_shape_translate_draw(&GSystem.pane, shape_table, shape_number, hotX, hotY);
        }
        else
        {
            // Draw normal
            VFX_shape_draw(pane, shape_table, shape_number, hotX, hotY);
        }
        // Draw 'A' letter for automated items
        if (Game.pCurPlanet->raceIndex == PlayerRaceIndex && pSquare->flags & ASCEND_SQUARE_FLAGS_AUTOMATED && extraInfo)
        {
            WinMgr.SmallFont.pane = pWnd->super.pane;
            DrawTextShaded(&WinMgr.SmallFont, hotX - 1, hotY + 16, "A", ASCEND_TF_HALIGN_CENTER_X | ASCEND_TF_VALIGN_CENTER_Y, 0xFFFF);
        }
    }
}

static void __declspec(naked) __cdecl Patch_DrawSurfaceItem_()
{
    _asm
    { 
        push    [esp+3A8h+4-48h]  // pSquare
        push    ebp  // pWnd
        call    Patch_DrawSurfaceItem
        add     esp, 8
        ret
    }
}

static void Patch_PlanetWndDrawListItemFunc(PANE* pPane, void* pData, int param1, BOOL hovered)
{
    char s[60];
    // ~ Original code
    UBYTE planetItemIndex = *(BYTE*)pData;
    P_PlanetItem pPlanetItem = &PlanetItems.item[planetItemIndex];
    P_Planet pPlanet = Game.pCurPlanet;

    bool isAlreadyBuilt = false;
    if (IsOneTimePlanetItem(planetItemIndex))
    {
        if (PlanetHasItem(Game.pCurPlanet, planetItemIndex))
        {
            isAlreadyBuilt = true;
        }
    }
    int color = isAlreadyBuilt ? 8 : 0xFFFF;

    VFX_pane_wipe(pPane, (hovered) ? 0x96 : 0xF2);
    if (planetItemIndex != 0xFF)
    {
        // T_Wnd07PW* pWnd07PW = (T_Wnd07PW*)WinMgr_FindWnd(&WinMgr, "PLANETALLOC", 0);
        UWORD cacheIndex, shapeNumber;
        Wnd07PW_GetItemShapeInfo(NULL, planetItemIndex, &cacheIndex, &shapeNumber);
        int scale = (planetItemIndex == ASCEND_PI_23_SHIP) ? 0x8000 : 0x10000;
        void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, cacheIndex);
        int hotX, hotY;
        Pane_CenterShape(pPane, shapeTable, shapeNumber, &hotX, &hotY);
        VFX_shape_transform(pPane, shapeTable, shapeNumber, 40, hotY, BigBuffer, 0, scale, scale, 0);
    }
    // Additions
    WinMgr.LargeFont.pane = *pPane;
    s[0] = 0;
    if (isAlreadyBuilt)
        strcpy(s, "(+) ");
    strcat(s, pPlanetItem->name);
    Font_DrawFormattedText(&WinMgr.LargeFont, 80, 0, s, ASCEND_TF_VALIGN_CENTER_PANE, color, 0xFF, 0);

    if (IsFixedCostPlanetItem(planetItemIndex))
    {
        WinMgr.SmallFont.pane = WinMgr.LargeFont.pane;
        // Days to build
        int cost = Planet_GetItemCost(pPlanet, 0xFFFF, planetItemIndex);
        int turns = GetTurnsToComplete(cost, pPlanet->buildingProgress, pPlanet->industry);
        sprintf(s, "%d d", turns);
        Font_DrawFormattedText(&WinMgr.LargeFont, 365, 0, s, ASCEND_TF_HALIGN_RIGHT_X | ASCEND_TF_VALIGN_CENTER_PANE, color, 0xFF, 0);
        // Cost
        int x1 = 80;
        int x2 = x1 + 45;
        int x3 = x2 + 20 * 2 + 35;
        int y = 44;
        sprintf(s, "%d", pPlanetItem->cost);
        Font_DrawFormattedText(&WinMgr.SmallFont, x1, y, s, 0, color, 0xFF, 0);
        // Resources
        if (pPlanetItem->res + pPlanetItem->ind + pPlanetItem->pros > 0)
        {
            sprintf(s, "%d", pPlanetItem->res);
            Font_DrawFormattedText(&WinMgr.SmallFont, x2, y, s, ASCEND_TF_HALIGN_CENTER_X, 109, 0xFF, 0);
            x2 += 20;
            sprintf(s, "%d", pPlanetItem->ind);
            Font_DrawFormattedText(&WinMgr.SmallFont, x2, y, s, ASCEND_TF_HALIGN_CENTER_X, 117, 0xFF, 0);
            x2 += 20;
            sprintf(s, "%d", pPlanetItem->pros);
            Font_DrawFormattedText(&WinMgr.SmallFont, x2, y, s, ASCEND_TF_HALIGN_CENTER_X, 146, 0xFF, 0);
        }
        // Orbital weapon
        if (IsOrbitalWeaponPlanetItem(planetItemIndex))
        {
            int rng = 0, dmg = 0, use = 0;
            Planet_GetOrbitalWeaponStats(NULL, planetItemIndex, &rng, &dmg, &use);
            sprintf(s, "dmg %d   rng %d   use %d", dmg, rng, use);
            Font_DrawFormattedText(&WinMgr.SmallFont, x2, y, s, 0, color, 0xFF, 0);
        }
        // + Maximum population
        if (pPlanetItem->mpop)
        {
            for (int i = 0; i < pPlanetItem->mpop; ++i)
            {
                VFX_shape_draw(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[44]), 3, x3, y - 1);
                x3 += 10;
            }
        }
    }
}

void __cdecl Patch_DrawOrbitalItem(P_Wnd07PW pWnd, int squareIndex, int savedRet, PANE* pane, void* shape_table, LONG shape_number, LONG hotX, LONG hotY, void* buffer, LONG rot, LONG x_scale, LONG y_scale, LONG flags)
{
    // Original code
    VFX_shape_transform(pane, shape_table, shape_number, hotX, hotY, buffer, rot, x_scale, y_scale, flags);
    // Draw 'A' letter for automated items
    bool extraInfo = Keyboard.Shift or (pWnd->displayFlags & 2);
    P_Square pSquare = &Game.pCurPlanet->pSquares[squareIndex];
    if (Game.pCurPlanet->raceIndex == PlayerRaceIndex && pSquare->flags & ASCEND_SQUARE_FLAGS_AUTOMATED && extraInfo)
    {
        WinMgr.SmallFont.pane = *pane;
        DrawTextShaded(&WinMgr.SmallFont, hotX, hotY, "A", ASCEND_TF_HALIGN_CENTER_X | ASCEND_TF_VALIGN_CENTER_Y, 0xFFFF);
    }
}

static void __declspec(naked) Patch_DrawOrbitalItem_()
{
    _asm
    {   
        push    [esp+3BCh+4-64h]  // squareIndex
        push    ebp  // pWnd
        call    Patch_DrawOrbitalItem
        add     esp, 08h
        ret
    }
}

void PatchPlanet::Hook(int stage)
{
    // Planet Window
    switch (stage)
    {
        case 0:
            WriteMemory(0x00038C82 + 1, Patch_DrawSurfaceItem_, false);
            WriteMemory(0x00038262 + 1, Patch_PlanetWndDrawListItemFunc, true);

            WriteMemory(0x00038F95 + 1, Patch_DrawOrbitalItem_, false);
            WriteMemory(0x0003924C + 1, Patch_DrawOrbitalItem_, false);

            break;
        case 1:
            // Planet Window
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "PLANETALLOC", 0);
            if (pWnd)
            {
                procs = Wnd07PW_Procs;
                procs.proc3_msg = (P_Proc3)Patch_WndPlanet_Proc3;
                procs.proc4_draw = (P_Proc4)Patch_WndPlanet_Proc4;
                pWnd->pProcs = &procs;
            }
    }
}

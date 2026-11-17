#include "patches/patch_planet_info_panel.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchPlanetInfoPanel)

//
// Type 29 "GSW" - Planet Research, Prosperity, Production, Industry, Population, Project
//

// =========================================================================
//  Constants
// =========================================================================
#define POP_ICONS_PER_ROW     10       // 0x0A - population icons per row
#define POP_ICON_SPACING      10       // 0x0A - spacing between icons
#define SHAPE_SCALE_NORMAL    0x10000  // 1.0 in 16.16 fixed-point
#define SHAPE_SCALE_SHIP      0x8000   // 0.5 - ships are drawn scaled down
#define WIPE_COLOR_ACTIVE     0x96
#define WIPE_COLOR_INACTIVE   0xF2
#define BORDER_COLOR_ACTIVE   0x96
#define TEXT_Y_ITEM_NAME      10  // 0x0A
#define TEXT_Y_PROJECT_STATUS 80
#define TEXT_Y_NO_FREE_POP    25

// Indexes in GwshareShpCacheIndexes
#define SHP_RESEARCH   0x26  // data\planres.shp
#define SHP_INDUSTRY   0x27  // data\planind.shp
#define SHP_PROSPERITY 0x28  // data\planpro.shp
#define SHP_POPULATION 0x2C  // data\17np.shp

// Static text indexes
#define TXT_NO_PROJECT            0x24  // "No Project"
#define TXT_PICK_LOCATION         0x25  // "Pick a Location"
#define TXT_DAY_SINGULAR          0x1C  // ""
#define TXT_DAY_PLURAL            0x1D  // "s"
#define TXT_DAYS_UNTIL_COMPLETION 0x26  // "%d day%s until completion"
#define TXT_NO_PROGRESS           0x27  // "No Progress"
#define TXT_NO_FREE_POPULATION    0x28  // "(No Free Population)"

// =========================================================================
//  Helper functions
// =========================================================================

// Draws population icons in a POP_ICONS_PER_ROW x N grid.
// The shape number switches at the boundaries of usedPopulation and population,
// which visually separates "used", "growing", and "free" population.
static void DrawPopulationIcons(PANE* pane, int maxPop, int usedPop, int currentPop)
{
    int shapeNumber = 1;
    int x = 0, y = 0;
    void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[SHP_POPULATION]);
    for (int i = 0; i < maxPop; ++i)
    {
        if (i == usedPop) ++shapeNumber;
        if (i == currentPop) ++shapeNumber;
        VFX_shape_draw(pane, shapeTable, (UWORD)shapeNumber, x, y);
        x += POP_ICON_SPACING;
        if (i % POP_ICONS_PER_ROW == POP_ICONS_PER_ROW - 1)
        {
            x = 0;
            y += POP_ICON_SPACING;
        }
    }
}

/*
// Draws a 3-pixel border around the pane.
static void DrawPaneBorder(PANE* pane, UBYTE color)
{
    int width = pane->x1 - pane->x0;
    int height = pane->y1 - pane->y0;

    for (int i = 0; i < 3; ++i)
    {
        Pane_CopyOrDrawRectangle(pane, i, i, width - i, height - i, color, 0);
    }
}
*/

// Determines if the window is active (under the cursor or forced) and not overlapped
// by auxiliary windows PLANLIST / PLSQUARE.
static bool IsWindowHovered(P_Wnd29GSW pWnd, int a2)
{
    if (a2 != 2 && (WinMgr.hotHwnd == pWnd->super.hwnd || a2 == 1))
    {
        T_Wnd10LB* planListWnd = (T_Wnd10LB*)WinMgr_FindWnd(&WinMgr, "PLANLIST", 0);
        T_Wnd14PSW* plSquareWnd = (T_Wnd14PSW*)WinMgr_FindWnd(&WinMgr, "PLSQUARE", 0);

        if (!planListWnd->super.shown && !plSquareWnd->super.shown)
            return TRUE;
    }
    return FALSE;
}

// Formats the current construction status string.
static void FormatProjectStatus(char* s, int days, UWORD flags)
{
    if ((flags & ASCEND_PI_FLAG_PROJECT) == 0)
    {
        if (days == 0xFFFF)
        {
            strcpy(s, StaticTxtRead(0x24));  // 36: "No Project"
        }
        else
        {
            sprintf(s, StaticTxtRead(0x26), days, StaticTxtRead((days == 1) ? 0x1C : 0x1D));  // 38: "%d day%s until completion"
        }
    }
}

static int GetPanelType(P_Wnd29GSW pWnd)
{
    // Select the display mode
    int panelType = pWnd->super.sendParam1;
    if (panelType == 4) panelType = 3;
    if (pWnd->super.sendMessage == 0x33) panelType = 4;
    return panelType;
}

static void DrawStatBar(PANE* pPane, int cacheId, int statValue, bool curve)
{
    int shapeNumber = (curve) ? (int)pow((double)statValue, 0.75) : statValue;
    if (shapeNumber > 0x14)
        shapeNumber = 0x14;
    UWORD shapeCacheIndex = GwshareShpCacheIndexes[cacheId];
    void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, shapeCacheIndex);
    VFX_shape_draw(pPane, shapeTable, (UWORD)shapeNumber, 0, 0);
}

static void Patch_AfterWnd29GSW_Proc4(P_Wnd29GSW pWnd);

// =========================================================================
//  Main window procedure
// =========================================================================
void Patch_Wnd29GSW_Proc4_v2(P_Wnd29GSW pWnd, int a2)
{
    bool isHovered = IsWindowHovered(pWnd, a2);
    int panelType = GetPanelType(pWnd);
    T_Wnd07PW* planetAllocWnd = (T_Wnd07PW*)WinMgr_FindWnd(&WinMgr, "PLANETALLOC", 0);
    PANE* pPane = &pWnd->super.pane;
    P_Planet pPlanet = Game.pCurPlanet;

    // VFX_shape_lookaside((*GSystem.pHazeTables)[0x15]);

    // Modes 0..2 clear the pane with a transparent wipe
    if (panelType <= 2)
        VFX_pane_wipe(pPane, 0);

    switch (panelType)
    {
        case 0:
            DrawStatBar(pPane, SHP_RESEARCH, pPlanet->research, true);
            break;
        case 1:
            DrawStatBar(pPane, SHP_INDUSTRY, pPlanet->industry, true);
            break;
        case 2:
            DrawStatBar(pPane, SHP_PROSPERITY, pPlanet->prosperity, false);
            break;
        case 3:
        {
            VFX_pane_wipe(pPane, isHovered ? WIPE_COLOR_ACTIVE : WIPE_COLOR_INACTIVE);
            // if (PlanetHasItem(pPlanet, ASCEND_PI_37_LUSH_GROWTH_BOMB))
            // {
            // UWORD cacheIndex, shapeNumber;
            // Wnd07PW_GetItemShapeInfo(NULL, ASCEND_PI_37_LUSH_GROWTH_BOMB, &cacheIndex, &shapeNumber);
            // VFX_shape_lookaside((*GSystem.pHazeTables)[gHazeNum]);
            // void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, cacheIndex);
            // int hotX, hotY;
            // Pane_CenterShape(pPane, shapeTable, shapeNumber, &hotX, &hotY);
            // VFX_shape_transform(&pWnd->super.pane, shapeTable, shapeNumber, 50 + gDX, 36 + gDY, BigBuffer, 0, 0x20000, 0x20000, 1);
            // }
            DrawPopulationIcons(pPane, pPlanet->maximumPopulation, pPlanet->usedPopulation, pPlanet->population);
            break;
        }

        // ----- Current project / construction -----
        case 4:
        {
            UBYTE wipeColor = (isHovered && pPlanet->raceIndex == PlayerRaceIndex) ? WIPE_COLOR_ACTIVE : WIPE_COLOR_INACTIVE;
            VFX_pane_wipe(pPane, wipeColor);

            char statusText[60];
            statusText[0] = 0;
            UBYTE itemIndex = 0xFF;
            UWORD buildingSquareIdx = 0xFFFF;
            const char* itemName = StaticTxtRead(TXT_NO_PROJECT);
            int textY = TEXT_Y_PROJECT_STATUS;
            BOOL drawStatusText = TRUE;

            UBYTE planetItemIdx = planetAllocWnd->planetItemIndex;

            if (planetItemIdx == 0xFF)
            {
                // The player is not selecting a square - show the planet's current project
                UBYTE buildingIdx = pPlanet->buildingPlanetItemIndex;
                if (buildingIdx != 0xFF)
                {
                    itemIndex = buildingIdx;
                    itemName = PlanetItems.item[buildingIdx].name;
                    FormatProjectStatus(statusText, Planet_GetDaysToCompleteItem(pPlanet), PlanetItems.item[buildingIdx].flags);
                    if (buildingIdx != ASCEND_PI_35_AUTOMATION)
                        buildingSquareIdx = pPlanet->buildingSquareIndex;
                }
            }
            else
            {
                // The player is selecting a square for placement
                itemIndex = planetItemIdx;
                itemName = PlanetItems.item[planetItemIdx].name;
                strcpy(statusText, StaticTxtRead(0x25));  // 37: "Pick a Location"
            }

            if (itemIndex == 0xFF)
            {
                // No project
                if (pPlanet->population == pPlanet->usedPopulation)
                {
                    // And no free population either
                    strcpy(statusText, StaticTxtRead(0x28));  // 40: "(No Free Population)"
                    textY = 25;
                }
                else
                {
                    drawStatusText = FALSE;  // do not draw status, only name
                }
            }
            else
            {
                // Draw the item shape centered in the pane
                UWORD shapeIdx, shapeNum;
                if (buildingSquareIdx == 0xFFFF)
                    Wnd07PW_GetItemShapeInfo(planetAllocWnd, itemIndex, &shapeIdx, &shapeNum);
                else
                    Wnd07PW_GetSquareShapeInfo(planetAllocWnd, buildingSquareIdx, &shapeIdx, &shapeNum);

                LONG scaleX = (itemIndex == ASCEND_PI_23_SHIP) ? SHAPE_SCALE_SHIP : SHAPE_SCALE_NORMAL;
                LONG scaleY = scaleX;

                LONG hotX, hotY;
                void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, shapeIdx);
                Pane_CenterShape(pPane, shapeTable, shapeNum, &hotX, &hotY);
                VFX_shape_transform(pPane, shapeTable, shapeNum, hotX, hotY,
                                    BigBuffer, 0, scaleX, scaleY, 0);
            }

            if (drawStatusText)
            {
                WinMgr.LargeFont.pane = *pPane;
                Font_DrawFormattedText(&WinMgr.LargeFont, 0, textY,
                                       statusText, 2u, 0xFFFF, 0xFF, 0);
            }

            WinMgr.LargeFont.pane = *pPane;
            Font_DrawFormattedText(&WinMgr.LargeFont, 0, TEXT_Y_ITEM_NAME,
                                   itemName, 2u, 0xFFFF, 0xFF, 0);
            break;
        }

        default:
            break;
    }

    // Border for modes 0..2 if the window is active and belongs to the player
    if (panelType <= 2 && isHovered && PlayerRaceIndex == pPlanet->raceIndex)
    {
        // DrawPaneBorder(pPane, BORDER_COLOR_ACTIVE);
        SCRNVERTEX vlist[4];
        PaneToQuad(*pPane, vlist);
        VFX_translate_polygon(&GSystem.pane, 4, vlist, (*GSystem.pHazeTables)[7]);
    }

    Patch_AfterWnd29GSW_Proc4(pWnd);

    WinMgr_AddDirtyAreaPane(&WinMgr, pPane);
}

static void Patch_AfterWnd29GSW_Proc4(P_Wnd29GSW pWnd)
{
    if (Game.pCurPlanet->raceIndex == PlayerRaceIndex && pWnd->super.sendMessage != 0x33)
    {
        int val, special;
        char s1[60];
        char s2[60];
        s1[0] = 0;
        s2[0] = 0;
        char* p1 = s1;
        char* p2 = s2;
        P_Planet pPlanet = Game.pCurPlanet;
        switch (pWnd->super.sendParam1)
        {
            case 0:
                val = pPlanet->research;
                special = ASCEND_PI_14_INTERNET;
                break;
            case 1:
                val = pPlanet->industry;
                special = ASCEND_PI_12_HYPERPOWER_PLANT;
                break;
            case 2:
                val = pPlanet->prosperity;
                special = ASCEND_PI_13_FERTILIZATION_PLANT;
                break;
            case 4:
                special = ASCEND_PI_37_LUSH_GROWTH_BOMB;
                break;
        }
        if (pWnd->super.sendParam1 != 4)
        {
            p1 += sprintf(p1, "%d", val);
        }
        if (PlanetHasItem(pPlanet, special))
        {
            p2 = stpcpy(p2, PlanetItems.item[special].name);
            // UWORD cacheIndex, shapeNumber;
            // Wnd07PW_GetItemShapeInfo(NULL, special, &cacheIndex, &shapeNumber);
            // void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, cacheIndex);
            // VFX_shape_transform(&pWnd->super.pane, shapeTable, shapeNumber, 17, 83, BigBuffer, 0, 0x8000, 0x8000, 0);
        }
        if (pWnd->super.sendParam1 == 2 && PlanetHasItem(pPlanet, ASCEND_PI_15_CLONING_PLANT))
        {
            p2 = stpcpy(p2, "\n");
            p2 = stpcpy(p2, PlanetItems.item[ASCEND_PI_15_CLONING_PLANT].name);
        }
        if (pWnd->super.sendParam1 == 2 && pPlanet->population < pPlanet->maximumPopulation)
        {
            p1 = stpcpy(p1, "\n");
            p1 += DaysText(p1, GetTurnsToComplete(50, pPlanet->growthProgress, pPlanet->prosperity));
        }
        WinMgr.SmallFont.pane = pWnd->super.pane;
        if (s1[0])
        {
            DrawTextShaded(&WinMgr.SmallFont, 0, 2, s1, ASCEND_TF_HALIGN_CENTER_PANE, 0xFFFF);
        }
        if (s2[0])
        {
            DrawTextShaded(&WinMgr.SmallFont, 0, 101, s2, ASCEND_TF_HALIGN_CENTER_PANE | ASCEND_TF_VALIGN_BOTTOM_Y, 0xD);
        }
    }
}

void Patch_Wnd29GSW_Proc4(P_Wnd29GSW pWnd, int a2)
{
    // Original draw
    Wnd29GSW_Procs.proc4_draw((P_WndA2)pWnd, a2);
    // Additional draw
    Patch_AfterWnd29GSW_Proc4(pWnd);
}

void PatchPlanetInfoPanel::Hook(int stage)
{
    // Planet Window - Top Info Panels
    switch (stage)
    {
        case 0:
            break;
        case 1:
            // Planet Window
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "PLANETALLOC", NULL);
            if (pWnd && pWnd->children)
            {
                // All Type 29 GSW children
                for (int i = 0; i < pWnd->childrenNum; ++i)
                {
                    P_WndA2 pChild = (*pWnd->children)[i];
                    if (pChild->pProcs == &Wnd29GSW_Procs)
                    {
                        procs = Wnd29GSW_Procs;
                        procs.proc4_draw = (P_Proc4)Patch_Wnd29GSW_Proc4_v2;
                        pChild->pProcs = &procs;
                    }
                }
            }
    }
}

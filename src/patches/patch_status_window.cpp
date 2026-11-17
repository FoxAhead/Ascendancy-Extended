#include "patches/patch_status_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchStatusWindow)

static UBYTE LutPow[56];
static int LutOrbitDx[10];
static int LutOrbitDy[10];

static void InitLuts()
{
    for (int i = 0; i < 56; ++i)
    {
        LutPow[i] = (UBYTE)pow((double)i, 0.75);
    }
    const double deg2rad = 0.01745329251994329576923690768489;
    const double radius = 58.0;
    int deg = 300;
    for (int i = 0; i < 10; ++i, deg -= 30)
    {
        double rad = (double)deg * deg2rad;
        LutOrbitDx[i] = (int)(radius * sin(rad));
        LutOrbitDy[i] = (int)(radius * cos(rad));
    }
}

// Utilities
static LONG GaugeShape(int v)
{
    return v > 55 ? LutPow[55] : (v > 0 ? LutPow[v] : 0);
}

static LONG ProsShape(int v)
{
    return v > 20 ? 20 : v;
}

// Item icon "like on the planet screen": for SHIP - racial smship, otherwise planitem.shp
static void DrawItemIcon(PANE* pPane, UBYTE item, int x, int y)
{
    int scale = 0x10000;
    UWORD cache, shape;
    if (item == ASCEND_PI_23_SHIP)
    {
        cache = GwshareShpCacheIndexes[PlayerRaceIndex + 14];  // smshipXX.shp
        shape = 0;
        scale = 0x8000;
    }
    else
    {
        cache = GwshareShpCacheIndexes[30];  // data\planitem.shp
        shape = item;
    }
    VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, cache), shape, x, y, BigBuffer, 0, scale, scale, 0);
}

// Common part: sun, planet, flag, names, self-managed, Building line
static int DrawPlanetItemHeader(PANE* pPane, P_Planet pPlanet, BOOL hovered)
{
    char s[128];
    P_Star pStar = &Game.stars[pPlanet->starIndex];
    int player = PlayerRaceIndex;
    UWORD raceCol = 4 * Game.races[player].colorSet + 0x13;
    int colStar = raceCol, colPlanet = raceCol;
    VFX_shape_draw(pPane, Cache_GetShapeTable(&WinMgr.cache, sunsShpCacheIndex2), pStar->type, 75, 75);
    VFX_shape_draw(pPane, Cache_GetShapeTable(&WinMgr.cache, planetsShpCacheIndex2), pPlanet->size + 5 * pPlanet->type, 200, 75);

    if (hovered)  // highlight "own" half of names
    {
        if (Mouse.x >= 150)
            colPlanet = 0xF3;
        else
            colStar = 0xF3;
    }

    if (pStar->planetsRacesBits == (1 << player) ||
        (pStar == Game.races[player].homeStar &&
         (pStar->planetsRacesBits & (1 << player)) != 0))
        DrawRaceFlagTinted(pPane, 4, 10, player);

    Font_DrawFormattedText(&WinMgr.SmallFont, 120, 18, pStar->name, 0, colStar, 0xFF, 0);
    Font_DrawFormattedText(&WinMgr.LargeFont, 275, 15, pPlanet->name, 0, colPlanet, 0xFF, 0);

    sprintf(s, StaticTxtRead(136), PlanetSizeTexts[pPlanet->size], PlanetTypeTexts[pPlanet->type]);  // "(%s %s Planet)"
    Font_DrawFormattedText(&WinMgr.SmallFont, 295 + Font_GetTextWidth(&WinMgr.LargeFont, pPlanet->name), 16, s, 0, colPlanet, 0xFF, 0);

    int hotY = 30;
    if (pPlanet->isSelfManaged == TRUE)
    {
        Font_DrawFormattedText(&WinMgr.SmallFont, 275, 30, StaticTxtRead(137), 0, colPlanet, 0xFF, 0);
        hotY = 39;
    }
    int projectColor = colPlanet;
    if (pPlanet->buildingPlanetItemIndex == 0xFF)
    {
        strcpy(s, StaticTxtRead(141));  // 141: "No Project"
        if (pPlanet->population == pPlanet->usedPopulation)
        {
            strcat(s, StaticTxtRead(142));  // 142: "   (No Free Population)"
        }
        projectColor = 0xF3;
    }
    else
    {
        char s2[60];
        s2[0] = '\0';
        if ((PlanetItems.item[pPlanet->buildingPlanetItemIndex].flags & 2) == 0)
        {
            int days = Planet_GetDaysToCompleteItem(pPlanet);
            if (days == 0xFFFF)
            {
                strcpy(s2, StaticTxtRead(138));  // 138: " - No Progress"
            }
            else
            {
                // 139: " - %d day%s until completion"
                sprintf(s2, StaticTxtRead(139), days, StaticTxtRead((days == 1) ? 0x1C : 0x1D));
            }
        }
        // 140: "Building %s%s"
        sprintf(s, StaticTxtRead(140), PlanetItems.item[pPlanet->buildingPlanetItemIndex].name, s2);
    }
    Font_DrawFormattedText(&WinMgr.SmallFont, 275, hotY, s, 0, projectColor, 0xFF, 0);
    return colPlanet;
}

// --- Customizable grid of the right block ---
#define SPEC_X     245  // finished special buildings, like in the original
#define SPEC_Y0    25
#define ROW_Y      56  // common row line
#define RES_X      280
#define IND_X      340
#define PRO_X      400
#define POP_X      460  // start of population figures
#define POP_STEP   5
#define POP_PERROW 10
#define PROJ_X     520  // NEW: current project

void Patch_StatusWndDrawListItemFunc(PANE* pPane, P_Planet pPlanet, int value, BOOL hovered)
{
    int i, n, x, y;
    char s[60];

    WinMgr.SmallFont.pane = *pPane;
    WinMgr.LargeFont.pane = *pPane;

    int color = DrawPlanetItemHeader(pPane, pPlanet, hovered);

    // Finished special buildings (original behavior)
    n = 9;
    for (i = 0; i < 2; ++i)
    {
        if (PlanetHasItem(pPlanet, SpecialDisplayItems[i]))
        {
            x = 200 + LutOrbitDx[n];
            y = 75 - LutOrbitDy[n];
            VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[30]), SpecialDisplayItems[i], x, y, BigBuffer, 0, 0x4000, 0x4000, 0);
            --n;
        }
    }

    // All other orbital buildings
    n = 0;
    for (i = pPlanet->surfaceSquaresNum; i < pPlanet->totalSquaresNum; ++i)
    {
        P_Square pSquare = &pPlanet->pSquares[i];
        if (pSquare->flags & ASCEND_SQUARE_FLAGS_COMPLETE)
        {
            int planetItemIndex = pSquare->planetItemIndex;
            if (planetItemIndex >= ASCEND_PI_25_ORBITAL_CLOAKER && planetItemIndex <= ASCEND_PI_30_LONG_RANGE_ORBITAL_WHOPPER)
            {
                x = 200 + LutOrbitDx[n];
                y = 75 - LutOrbitDy[n];
                VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[30]), planetItemIndex, x, y, BigBuffer, 0, 0x4000, 0x4000, 0);
                ++n;
            }
        }
    }

    // tightly packed row of gauges: res / ind / pro
    VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[38]), GaugeShape(pPlanet->research), RES_X, ROW_Y, BigBuffer, 0, 0x8000, 0x8000, 0);
    VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[39]), GaugeShape(pPlanet->industry), IND_X, ROW_Y, BigBuffer, 0, 0x8000, 0x8000, 0);
    VFX_shape_transform(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[40]), ProsShape(pPlanet->prosperity), PRO_X, ROW_Y, BigBuffer, 0, 0x8000, 0x8000, 0);

    // population right after prosperity, in two rows
    int shape_number = 6;
    for (i = 0; i < pPlanet->maximumPopulation; ++i)
    {
        if (i == pPlanet->usedPopulation)
            shape_number = 7;
        if (i == pPlanet->population)
            shape_number = 8;
        int k = i % POP_PERROW, r = i / POP_PERROW;
        VFX_shape_draw(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[44]), shape_number, POP_X + POP_STEP * k, ROW_Y + POP_STEP * r);
    }

    // FIFTH element: current project (icon of the building under construction)
    if (pPlanet->buildingPlanetItemIndex != 0xFF)
    {
        DrawItemIcon(pPane, pPlanet->buildingPlanetItemIndex, PROJ_X + 28, ROW_Y + 17);
    }

    sprintf(s, "%d", pPlanet->research);
    DrawTextShaded(&WinMgr.SmallFont, RES_X + 26, ROW_Y + 1, s, 8, color);
    sprintf(s, "%d", pPlanet->industry);
    DrawTextShaded(&WinMgr.SmallFont, IND_X + 26, ROW_Y + 1, s, 8, color);
    sprintf(s, "%d", pPlanet->prosperity);
    DrawTextShaded(&WinMgr.SmallFont, PRO_X + 26, ROW_Y + 1, s, 8, color);
    if (pPlanet->population < pPlanet->maximumPopulation && DaysText(s, GetTurnsToComplete(50, pPlanet->growthProgress, pPlanet->prosperity)))
    {
        DrawTextShaded(&WinMgr.SmallFont, PRO_X + 26, ROW_Y + 51, s, 0x18, color);
    }
}

void PatchStatusWindow::Hook(int stage)
{
    // Status Window (planets list)
    switch (stage)
    {
        case 0:
            WriteMemory(0x00051934 + 1, Patch_StatusWndDrawListItemFunc, true);
            break;
        case 1:
            InitLuts();
            break;
    }
}

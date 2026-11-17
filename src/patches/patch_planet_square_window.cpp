#include "patches/patch_planet_square_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchPlanetSquareWindow)

static const int SquareColor[3] = {ASCEND_SQUARE_COLOR_BLUE, ASCEND_SQUARE_COLOR_RED, ASCEND_SQUARE_COLOR_GREEN};
static const int MultItem[3] = {ASCEND_PI_14_INTERNET, ASCEND_PI_12_HYPERPOWER_PLANT, ASCEND_PI_13_FERTILIZATION_PLANT};
static const int Project[3] = {ASCEND_PI_34_SCIENTIST_TAKEOVER, -1, ASCEND_PI_33_ENDLESS_PARTY};
static const int Offset[3] = {1, 0, 2};

struct T_Resource
{
    int itemsCount[39];
    int itemsPoints[39];
    const char* pMultItemName;
    const char* pProjItemName;
    int raw, mult, proj, curve, support;
};

static void CalcPlanetPoints(P_Planet pPlanet, T_Resource pData[3])
{
    memset(pData, 0, sizeof(T_Resource) * 3);
    // Raw
    for (int i = 0; i < pPlanet->totalSquaresNum; ++i)
    {
        P_Square pSquare = &pPlanet->pSquares[i];
        int planetItemIndex = pSquare->planetItemIndex;
        if (planetItemIndex != 0xFF && (pSquare->flags & ASCEND_SQUARE_FLAGS_COMPLETE) != 0)
        {
            const UBYTE* pVal = &PlanetItems.item[planetItemIndex].ind;
            for (int j = 0; j < 3; ++j)
            {
                T_Resource* pResource = &pData[j];
                int val = pVal[Offset[j]];
                if (val)
                {
                    int points = val + (pSquare->color == SquareColor[j] ? 1 : 0);
                    ++pResource->itemsCount[planetItemIndex];
                    pResource->itemsPoints[planetItemIndex] += points;
                    pResource->raw += points;
                }
                if (planetItemIndex == MultItem[j])
                {
                    pResource->pMultItemName = PlanetItems.item[planetItemIndex].name;
                }
            }
        }
    }
    // Mult
    for (int j = 0; j < 3; ++j)
    {
        T_Resource* pResource = &pData[j];
        pResource->mult = pResource->raw + (pResource->pMultItemName ? pResource->raw >> 1 : 0);
    }
    for (int j = 0; j < 3; ++j)
    {
        T_Resource* pResource = &pData[j];
        // Proj
        pResource->proj = pResource->mult;
        if (j != 1 && pPlanet->buildingPlanetItemIndex == Project[j])
        {
            pResource->pProjItemName = PlanetItems.item[pPlanet->buildingPlanetItemIndex].name;
            pResource->proj += pData[1].mult >> 2;
        }
        // Curve
        pResource->curve = j ? (int)pow((double)(pResource->proj + 1), 0.85) : pResource->proj;
        // Support
        pResource->support = pResource->curve;
        if (j == 2)
        {
            pResource->support -= (pPlanet->population >> 2);
            if (pResource->support < 0)
            {
                pResource->support = 0;
            }
        }
    }
}

static int CalcFromRaw(int raw, bool mult, int projInd)
{
    raw += (mult ? raw >> 1 : 0) + projInd;
    return (int)pow((double)(raw + 1), 0.85);
}

void __stdcall Patch_Wnd14PSW_Proc4_DrawSecondText2()
{
    if (PlSquareWndMode < 0 || PlSquareWndMode > 2)
        return;
    T_Resource data[3];
    P_Planet pPlanet = Game.pCurPlanet;
    CalcPlanetPoints(pPlanet, data);
    T_Resource* pResource = &data[PlSquareWndMode];

    char s1[1024];
    char s2[1024];
    char s3[1024];
    s1[0] = 0;
    s2[0] = 0;
    s3[0] = 0;
    char* p1 = s1;
    char* p2 = s2;
    char* p3 = s3;
    for (int i = 0; i < 39; ++i)
    {
        if (pResource->itemsCount[i])
        {
            p1 += sprintf(p1, "\n%d", pResource->itemsCount[i]);
            p2 += sprintf(p2, "\n%s", PlanetItems.item[i].name);
            p3 += sprintf(p3, "\n= %d", pResource->itemsPoints[i]);
        }
    }
    p1 += sprintf(p1, "\n");
    p2 += sprintf(p2, "\nTotal");
    p3 += sprintf(p3, "\n= %d", pResource->raw);
    if (pResource->pMultItemName)
    {
        p1 += sprintf(p1, "\n");
        p2 += sprintf(p2, "\n%s: %d x 1.5", pResource->pMultItemName, pResource->raw);
        p3 += sprintf(p3, "\n= %d", pResource->mult);
    }
    if (pResource->pProjItemName)
    {
        p1 += sprintf(p1, "\n");
        p2 += sprintf(p2, "\n%s: %d + %d / 4", pResource->pProjItemName, pResource->mult, data[1].mult);
        p3 += sprintf(p3, "\n= %d", pResource->proj);
    }
    if (PlSquareWndMode != 0)
    {
        p1 += sprintf(p1, "\n");
        p2 += sprintf(p2, "\nPow(%d + 1, 0.85)", pResource->proj);
        p3 += sprintf(p3, "\n= %d", pResource->curve);
    }
    int y = 85;
    int yn = y + GetMultiLineTextHeight(&WinMgr.SmallFont, s1 + 1);
    char sn[256];
    sn[0] = 0;
    char* pn = sn;
    if (PlSquareWndMode != 0)
    {
        pn += sprintf(pn, "(Next:");
        int raw = pResource->raw;
        int projInd = pResource->pProjItemName ? data[1].mult >> 2 : 0;
        for (int i = 0; i < 10; ++i)
        {
            // int next = (int)pow((double)(++pResource->proj + 1), 0.85);
            int next = CalcFromRaw(++raw, pResource->pMultItemName, projInd);
            pn += sprintf(pn, " %d", next);
        }
        pn += sprintf(pn, ")");
    }
    if (PlSquareWndMode == 2)
    {
        p1 += sprintf(p1, "\n\n");
        p2 += sprintf(p2, "\n\nPopulation: %d - %d / 4", pResource->curve, pPlanet->population);
        p3 += sprintf(p3, "\n\n= %d", pResource->support);
    }

    WinMgr.SmallFont.pane = WinMgr.LargeFont.pane;
    int w1 = GetMultiLineTextWidth(&WinMgr.SmallFont, s1 + 1);
    int w2 = GetMultiLineTextWidth(&WinMgr.SmallFont, s2 + 1);
    int w3 = GetMultiLineTextWidth(&WinMgr.SmallFont, s3 + 1);
    int h = GetMultiLineTextHeight(&WinMgr.SmallFont, s1 + 1);
    int w = w1 + w2 + w3;
    int x = ((WinMgr.SmallFont.pane.x1 - WinMgr.SmallFont.pane.x0) - w - 20) / 2;
    // x += w1;
    Font_DrawFormattedText(&WinMgr.SmallFont, x, y, s1 + 1, 0, 0xFFFF, 0xFF, 0);
    x += w1 + 10;
    Font_DrawFormattedText(&WinMgr.SmallFont, x, y, s2 + 1, 0, 0xFFFF, 0xFF, 0);
    x += 10 + w2;
    Font_DrawFormattedText(&WinMgr.SmallFont, x, y, s3 + 1, 0, 0xFFFF, 0xFF, 0);

    if (sn[0])
    {
        Font_DrawFormattedText(&WinMgr.SmallFont, 0, yn, sn, ASCEND_TF_HALIGN_CENTER_PANE, 0xFFFF, 0xFF, 0);
    }
}

static void Patch_Wnd14PSW_Proc4(P_Wnd14PSW pWnd, int a2)
{
    int savedBackColor = WinMgr.LargeFont.backColor;
    Wnd14PSW_Procs.proc4_draw((P_WndA2)pWnd, a2);
    WinMgr.LargeFont.backColor = savedBackColor;
    WinMgr.LargeFont.pane = pWnd->super.pane;
    WinMgr.SmallFont.pane = pWnd->super.pane;
    Patch_Wnd14PSW_Proc4_DrawSecondText2();
}

void PatchPlanetSquareWindow::Hook(int stage)
{
    // Planet Square Window
    switch (stage)
    {
        case 0:
            // WriteMemory(0x0003B0FA + 1, Patch_Wnd14PSW_Proc4_DrawSecondText_, false);
            WriteMemory(0x0003B0EC + 1, NULL, false, 50);
            break;
        case 1:
            P_WndA2 pWnd = WinMgr_FindWnd(&WinMgr, "PLSQUARE", 0);
            if (pWnd)
            {
                procs = Wnd14PSW_Procs;
                procs.proc4_draw = (P_Proc4)Patch_Wnd14PSW_Proc4;
                pWnd->pProcs = &procs;
            }
            break;
    }
}

#include "patches/patch_battle_control_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchBattleControlWindow)

static void Patch_BatControlWndDrawListItemFuncGizmo(PANE* pPane, void* pData, int ps, BOOL hovered)
{
    /*
    char s[68];
    LONG hotX, hotY;

    P_HullCell pHullCell = (P_HullCell)pData;
    void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[0x1C]);  // 28: data\gizmos.shp
    if (hovered && !ps && (Gizmos[pHullCell->gizmoIndex].flags & ASCEND_GZ_PASSIVE) == 0)
    {
        VFX_pane_wipe(pPane, 0x96);
    }
    Pane_CenterShape(pPane, shapeTable, pHullCell->gizmoIndex, &hotX, &hotY);
    if (ps == 2 || ps == 3)
    {
        VFX_shape_lookaside((*GSystem.pHazeTables)[7]);
        VFX_shape_translate_draw(pPane, shapeTable, pHullCell->gizmoIndex, hotX, hotY);
    }
    else
    {
        VFX_shape_draw(pPane, shapeTable, pHullCell->gizmoIndex, hotX, hotY);
    }
    int color = 4 * Game.races[PlayerRaceIndex].colorSet + 0x13;
    if (pHullCell->active)
    {
        if ((Gizmos[pHullCell->gizmoIndex].flags & ASCEND_GZ_TOGGLEABLE) != 0)
        {
            Pane_CopyOrDrawRectangle(pPane, 1, 1, pPane->x1 - pPane->x0 - 1, pPane->y1 - pPane->y0 - 1, color, 0);
        }
    }
    sprintf(s, "%s", Gizmos[pHullCell->gizmoIndex].name);
    WinMgr.SmallFont.pane = *pPane;
    Font_DrawFormattedText(&WinMgr.SmallFont, 5, 2, s, 0, color, 0xFF, 0);
    if (ps == 2 || ps == 3)
    {
        int v8 = (ps == 3);  // 0: "(depleted)", 1: "(insufficient power)"
        Font_DrawFormattedText(&WinMgr.SmallFont, 5, 0xC, StaticTxtRead(v8), 0, 0xF, 0xFF, 0);
    }
    */
    BatControlWndDrawListItemFuncGizmo(pPane, pData, ps, hovered);
    P_HullCell pHullCell = (P_HullCell)pData;
    P_Gizmo pGizmo = &Gizmos[pHullCell->gizmoIndex];
    char s1[20], s2[20];
    GetGizmoExtraInfo(pGizmo, s1, s2);
    if (s1[0])
        Font_DrawFormattedText(&WinMgr.SmallFont, 5, 31, s1, 0x0, 0x3C, 0xFF, 122);
    if (s2[0])
        Font_DrawFormattedText(&WinMgr.SmallFont, 132, 31, s2, ASCEND_TF_HALIGN_RIGHT_X, 0x3C, 0xFF, 122);
}

static void Patch_BatControlWndDrawListItemFuncOrbital(PANE* pPane, P_Square pSquare, int depleted, BOOL hovered)
{
    BatControlWndDrawListItemFuncOrbital(pPane, pSquare, depleted, hovered);
    int left = (int)(pSquare->flags >> 8);
    int rng = 0, dmg = 0, use = 0;
    Planet_GetOrbitalWeaponStats(NULL, pSquare->planetItemIndex, &rng, &dmg, &use);
    char s1[20];
    sprintf(s1, "dmg %d\nrng %d\nuse %d/%d", dmg, rng, left, use);
    Font_DrawFormattedText(&WinMgr.SmallFont, 2, 31, s1, 0x0, 0x3C, 0xFF, 122);
}

void PatchBattleControlWindow::Hook(int stage)
{
    // Battle Control Window
    switch (stage)
    {
        case 0:
            WriteMemory(0x00011D59 + 1, Patch_BatControlWndDrawListItemFuncGizmo, true);
            WriteMemory(0x00011F19 + 1, Patch_BatControlWndDrawListItemFuncOrbital, true);
            break;
    }
}

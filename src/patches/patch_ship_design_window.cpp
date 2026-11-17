#include "patches/patch_ship_design_window.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchShipDesignWindow)

void Patch_ShipDesignWndDrawListItemFunc(PANE* pPane, void* pData, int param1, BOOL hovered)
{
    if (hovered)
    {
        VFX_pane_wipe(pPane, 0x96);
    }
    VFX_shape_draw(pPane, Cache_GetShapeTable(&WinMgr.cache, GwshareShpCacheIndexes[0x1C]), param1, 63, 45);
    P_Gizmo pGizmo = &Gizmos[param1];
    WinMgr.SmallFont.pane = *pPane;
    Font_DrawFormattedText(&WinMgr.SmallFont, 63, 2, pGizmo->name, ASCEND_TF_HALIGN_CENTER_X, 0xFFFF, 0xFF, 122);
    char s1[20], s2[20];
    GetGizmoExtraInfo(pGizmo, s1, s2);
    if (s1[0])
        Font_DrawFormattedText(&WinMgr.SmallFont, 0, 24, s1, 0x0, 0x3C, 0xFF, 122);
    if (s2[0])
        Font_DrawFormattedText(&WinMgr.SmallFont, 127, 24, s2, 0x20, 0x3C, 0xFF, 122);
}

void PatchShipDesignWindow::Hook(int stage)
{
    // Ship Design Screen
    switch (stage)
    {
        case 0:
            WriteMemory(0x0004DFDF + 1, Patch_ShipDesignWndDrawListItemFunc, true);
            break;
    }
}

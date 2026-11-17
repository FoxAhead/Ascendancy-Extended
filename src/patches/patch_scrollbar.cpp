#include "patches/patch_scrollbar.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "utils.h"

REGISTER_PATCH(PatchScrollBar)

struct SB_GEOM
{
    int x0, trackY0, trackY1, thumbY0, thumbY1;
};

static void SB_GetGeom(P_Wnd10LB lb, SB_GEOM* g)
{
    PANE* p = &lb->super.pane;
    int h = p->y1 - p->y0;
    int btn = lb->scrollBarWidth;
    int trackHeight, thumbH, total, visible, maxScroll;

    g->x0 = p->x1 - btn;           // Global X: scrollbar column
    g->trackY0 = p->y0 + 1 + btn;  // Y is already global (pane.y0 is global)
    g->trackY1 = p->y1 - 1 - btn;
    trackHeight = g->trackY1 - g->trackY0;

    total = lb->itemCount;
    if (total <= 0) total = 1;
    visible = h / lb->_defaultItemHeight;
    if (visible > total) visible = total;
    thumbH = (trackHeight * visible) / total;
    if (thumbH < 10) thumbH = 10;
    if (thumbH > trackHeight) thumbH = trackHeight;

    maxScroll = total - visible;
    g->thumbY0 = g->trackY0;
    if (maxScroll > 0)
        g->thumbY0 += (lb->topIndex * (trackHeight - thumbH)) / maxScroll;
    g->thumbY1 = g->thumbY0 + thumbH;
}

static void Patch_Wnd10LB_DrawScrollBar(P_Wnd10LB pListBox, int refresh)
{
    PANE* pPane = &pListBox->super.pane;

    int x0 = pPane->x1 - pPane->x0 - pListBox->scrollBarWidth;
    int h = pPane->y1 - pPane->y0;
    VFX_line_draw(pPane, x0, 0, x0, h, 0, pListBox->_borderColor);

    PANE pane = *pPane;
    pane.x0 = pPane->x1 - pListBox->scrollBarWidth;
    InflatePane(&pane, -1, -1);
    VFX_pane_wipe(&pane, pListBox->wipeColor1);

    PANE pane1 = pane;
    if (pListBox->scrollThumbState > 1u)
    {
        int centerLineY = pPane->y0 + h / 2;
        if (pListBox->scrollThumbState == 2)
        {
            pane1.y1 = centerLineY;
        }
        else
        {
            pane1.y0 = centerLineY;
        }
        VFX_pane_wipe(&pane1, 0x96);
    }

    // Draw 2 buttons
    int hotX = x0 + 2;
    void* shapeTable = Cache_GetShapeTable(&WinMgr.cache, pListBox->super.shapeCacheIndex);
    VFX_shape_draw(pPane, shapeTable, pListBox->super.frameShapeNumber, hotX, 2);
    int hotY = h - pListBox->scrollBarWidth + 2;
    shapeTable = Cache_GetShapeTable(&WinMgr.cache, pListBox->super.shapeCacheIndex);
    VFX_shape_draw(pPane, shapeTable, pListBox->super.frameShapeNumber + 1, hotX, hotY);

    // 3. Calculate available height for thumb movement
    // Exclude space occupied by top and bottom arrows
    int btnHeight = pListBox->scrollBarWidth;  // Usually arrows are square
    int trackY0 = pane.y0 + btnHeight;
    int trackY1 = pane.y1 - btnHeight;
    int trackHeight = trackY1 - trackY0;

    // Default values for safety (if few items or data is uninitialized)
    int total = pListBox->itemCount;
    int visible = h / pListBox->_defaultItemHeight;
    int current = pListBox->topIndex;

    if (total <= 0) total = 1;
    if (visible > total) visible = total;

    // 4. Calculate thumb size
    // Thumb height is proportional to the percentage of visible items
    int thumbHeight = (trackHeight * visible) / total;

    // Set minimum thumb height (e.g., 10 pixels) so it doesn't shrink to a dot
    if (thumbHeight < 10) thumbHeight = 10;
    if (thumbHeight > trackHeight) thumbHeight = trackHeight;

    // 5. Calculate thumb position
    int thumbY0 = trackY0;
    int maxScroll = total - visible;

    if (maxScroll > 0)
    {
        // How many pixels of "free travel" are left for the thumb in the track
        int freeTrackSpace = trackHeight - thumbHeight;
        // Offset the thumb proportionally to the current scroll index
        thumbY0 += (current * freeTrackSpace) / maxScroll;
    }
    int thumbY1 = thumbY0 + thumbHeight;

    // 6. Draw the thumb
    // Set the boundaries of the temporary pane to the thumb size
    PANE thumbPane = pane;
    InflatePane(&thumbPane, -pListBox->scrollBarWidth / 8, 0);
    thumbPane.y0 = thumbY0;
    thumbPane.y1 = thumbY1;

    // Draw the thumb itself
    VFX_pane_wipe(&thumbPane, 103);

    if (refresh == TRUE)
    {
        WinMgr_AddDirtyArea(&WinMgr, x0 + pPane->x0, pPane->y0, pPane->x1, pPane->y1);
    }
}

static int GetMaxScroll(P_Wnd10LB lb)
{
    int visible = (lb->super.pane.y1 - lb->super.pane.y0) / lb->_defaultItemHeight;
    int maxScroll = (int)lb->itemCount - visible;
    if (maxScroll < 0) maxScroll = 0;
    return maxScroll;
}

#define WM_LB_DRAGRELEASE 0x196

int Patch_Wnd10LB_Proc3(P_Wnd10LB lb, UWORD message, int param1, int param2)
{
    SB_GEOM g;
    int trackHeight, thumbH, free, total, visible, maxScroll, y, idx;

    switch (message)
    {
        case ASCEND_WM_KEYBOARD:
            if (lb->super.shown && lb->itemCount)
            {
                int newPos = 0;
                switch (param2)
                {
                    case KSC_NUMPAD7:  // Home
                    case KSC_NUMPAD1:  // End
                        newPos = (param2 == KSC_NUMPAD7) ? 0 : GetMaxScroll(lb);
                        if (lb->topIndex != (UWORD)newPos)
                        {
                            lb->topIndex = (UWORD)newPos;
                            lb->hoveredItemIndex = 0xFFFF;
                            lb->super.pProcs->proc4_draw(&lb->super, 0);
                        }
                        return TRUE;
                    case KSC_NUMPAD9:  // PgUp
                    case KSC_NUMPAD3:  // PgDn
                                       // if (WinMgr._currentHwnd == lb->super.hwnd)
                                       // {
                        lb->hoveredItemIndex = 0xFFFF;
                        lb->super.pProcs->proc3_msg((P_WndA2)lb, (param2 == KSC_NUMPAD9) ? ASCEND_WM_LIST_SCROLLUP : ASCEND_WM_LIST_SCROLLDOWN, 0, 0);
                        return TRUE;
                        // }
                }
            }
            return WndA2_Proc3(&lb->super, message, param1, param2);

        case ASCEND_WM_LEFTCLICK:  // Left mouse button click
            if (lb->hasScrollBar == TRUE && param1 >= lb->super.pane.x1 - lb->scrollBarWidth)
            {
                SB_GetGeom(lb, &g);
                if (param2 >= g.thumbY0 && param2 < g.thumbY1)
                {
                    lb->Unknown_B7 = TRUE;  // drag is active
                    lb->Unknown_BB = param2 - g.thumbY0;
                    // Small f=1: will fire on the release front
                    WinMgr_AddInputTrigger(&WinMgr, lb->super.hwnd, 1, WM_LB_DRAGRELEASE, 0, 0);
                    return 0xFFFFFFFF;  // do not pass to sub_2F090 (otherwise autoscroll will activate)
                }
            }
            break;

        case ASCEND_WM_MOUSEMOVE:  // movement (global coordinates)
            if (lb->Unknown_B7 == TRUE)
            {
                SB_GetGeom(lb, &g);
                trackHeight = g.trackY1 - g.trackY0;
                thumbH = g.thumbY1 - g.thumbY0;
                free = trackHeight - thumbH;

                y = param2 - lb->Unknown_BB;
                if (y < g.trackY0) y = g.trackY0;
                if (y > g.trackY1 - thumbH) y = g.trackY1 - thumbH;

                total = lb->itemCount;
                visible = (lb->super.pane.y1 - lb->super.pane.y0) / lb->_defaultItemHeight;
                maxScroll = total - visible;
                if (maxScroll < 0) maxScroll = 0;

                idx = (free > 0 && maxScroll > 0)
                          ? ((y - g.trackY0) * maxScroll + free / 2) / free
                          : 0;
                if (idx > maxScroll) idx = maxScroll;

                if (idx != lb->topIndex)
                {
                    lb->topIndex = (UWORD)idx;
                    lb->super.pProcs->proc4_draw(&lb->super, 0);
                }
                return 0xFFFFFFFF;
            }
            break;

        case ASCEND_WM_MOUSELEAVE:       // focus lost: during drag, do not pass to the original handler
            if (lb->Unknown_B7 == TRUE)  // (otherwise it will wipe winEventSmall f=4 and movement will freeze)
                return 0xFFFFFFFF;
            break;

        case WM_LB_DRAGRELEASE:  // release front from small f=1
            if (lb->Unknown_B7 == TRUE)
            {
                lb->Unknown_B7 = 0;
                WinMgr_RemoveInputTrigger(&WinMgr, lb->super.hwnd, WM_LB_DRAGRELEASE);
                lb->super.pProcs->proc4_draw(&lb->super, 0);
                // if the cursor remained outside the pane - manually complete the focus cleanup,
                // which we suppressed on message 6:
                if (param1 /*lastX*/ || param2)
                {  // see note below
                }
            }
            return 0xFFFFFFFF;

        case ASCEND_WM_DEACTIVATE:  // window hiding - safety fallback
            if (lb->Unknown_B7 == TRUE)
            {
                lb->Unknown_B7 = 0;
                WinMgr_RemoveInputTrigger(&WinMgr, lb->super.hwnd, WM_LB_DRAGRELEASE);
            }
            break;
    }
    return Wnd10LB_Proc3(lb, message, param1, param2);
}

void PatchScrollBar::Hook(int stage)
{
    switch (stage)
    {
        case 0:
            WriteMemory(0x0002E776, Patch_Wnd10LB_DrawScrollBar, false, OP_CALL);
            WriteMemory(0x0002EEE8, Patch_Wnd10LB_DrawScrollBar, false, OP_CALL);
            WriteMemory(0x0002F082, Patch_Wnd10LB_DrawScrollBar, false, OP_CALL);

            Wnd10LB_Procs.proc3_msg = (P_Proc3)Patch_Wnd10LB_Proc3;
            break;
    }
}

#include "patches/patch_hotkeys.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"

REGISTER_PATCH(PatchHotkeys)

struct HelpItem
{
    const char* key;
    const char* desc;
};

static const char DEBUG[] = "DEBUG";

// OPENING
static const HelpItem Screen0[] = {
    {"Alt+N", "Sound"},
    {"Alt+M", "Music"},
    {"Alt+L", "Load Game"},
    {"Alt+S", "Save Game"},
    {"Alt+X", "Exit to DOS"},
    {"Alt+G", "Save GIF screenshot"},
    {"PgUp/PgDn/Home/End", "Scroll lists"},
    {DEBUG, DEBUG},
    {"Alt+W", "Debug window"},
    {"Alt+D", "Show mouse coordinates"},
    {"Alt+F", "Show FPS"},
    {"Alt+E", "Toggle research stealing mode"},
    {"Alt+R", "Record WINEVENT"},
    {"Alt+P", "Play WINEVENT"},
    {NULL, NULL},
};

// MAIN_SCREEN
static const HelpItem Screen1[] = {
    {"Enter", "Advance one day"},
    {"F2, F3, F4, F5, F6", "Planets, Ships, Research, Ability, Species"},
    {"N or hold Shift", "Display star names"},
    {"Left click", "Star - go to system, Ship - go to ship"},
    {"Right click", "Star - select, Ship - open contents"},
    {"- / =", "Decrease / increase rotation speed"},
    {DEBUG, DEBUG},
    {"1, 2, 3, 4, 5, 6, 7", "Play as other species"},
    {"8", "See everything"},
    {"C", "Attitudes and treaties"},
    {"S", "All stars explored"},
    {"L", "All starlanes explored"},
    {"R", "Reset treaties and attitudes "},
    {NULL, NULL},
};

// PLANET_STATUS
static const HelpItem Screen10[] = {
    {"PgUp/PgDn/Home/End", "Scroll planets list"},
    {NULL, NULL},
};

// BATTLE
static const HelpItem Screen16[] = {
    {"Space or Enter", "Skip animation"},
    {"- / =", "Decrease / increase rotation speed"},
    {NULL, NULL},
};

// PLANET_ALLOC
static const HelpItem Screen17[] = {
    {"C", "Colored surface items"},
    {"H", "Surface items"},
    {"P", "Planet picture"},
    {"S", "Squares"},
    {"M", "Toggle self-managed"},
    {"Hold Shift", "Additional info"},
    {"PgUp/PgDn/Home/End", "Scroll items list"},
    {DEBUG, DEBUG},
    {"R", "Take over planet"},
    {"T", "Create Colony Base"},
    {"I", "Build current project"},
    {"Shift+M", "Increase maximum population"},
    {"O", "Fill population up to maximum"},
    {"N", "Process planet one turn"},
    {NULL, NULL},
};

// FLEET_VIEW
static const HelpItem Screen18[] = {
    {"PgUp/PgDn/Home/End", "Scroll ships list"},
    {"Left click", "Go to ship"},
    {"Right click", "Open ship contents"},
    {NULL, NULL},
};

// RESEARCH
static const HelpItem Screen20[] = {
    {"M", "Toggle self-managed"},
    {"PgUp/PgDn/Home/End", "Scroll research tree"},
    {"- / =", "Decrease / increase rotation speed"},
    {DEBUG, DEBUG},
    {"D", "Discover everything"},
    {"F", "Forget everything"},
    {"C", "Complete current research"},
    {NULL, NULL},
};

static const HelpItem* const HelpGuide[21] = {
    Screen0,   //  0 OPENING
    Screen1,   //  1 MAIN_SCREEN
    NULL,      //  2 SAVE_GAME
    NULL,      //  3 NEW_GAME
    NULL,      //  4 LOAD_GAME
    NULL,      //  5
    NULL,      //  6 INTRO_FLIC
    NULL,      //  7 RACE_SELECT
    NULL,      //  8 SHIP_DESIGN
    NULL,      //  9 ENDGAME
    Screen10,  // 10 PLANET_STATUS
    NULL,      // 11
    NULL,      // 12 STARTGAME
    NULL,      // 13 DIPLOMACY
    NULL,      // 14 INTEL
    NULL,      // 15 TUTORIAL
    Screen16,  // 16 BATTLE
    Screen17,  // 17 PLANET_ALLOC
    Screen18,  // 18 FLEET_VIEW
    NULL,      // 19
    Screen20,  // 20 RESEARCH
};

static void AddTextItem(P_Wnd20HW pHelpWnd, char* text, P_Font pFont, int x, int y, int flags)
{
    P_HelpItem it = &pHelpWnd->helpItems[pHelpWnd->helpItemsNum++];
    it->type = 1;
    it->pText = text;
    it->pFont = pFont;
    it->x = x;
    it->y = y;
    it->flags = flags;
    it->color = 0xFFFF;
    it->hasNextPage = 0;
}

void ShowHotkeyHelpMem(P_WinMgr pWinMgr, P_Wnd20HW pHelpWnd, int st)
{
    pHelpWnd->helpItemsNum = 0;
    pHelpWnd->pagesNum = 0;
    pHelpWnd->page = 0;
    sprintf(pHelpWnd->helpTopic, "hotkey%02d", st);
    int x0 = (st == 0 || st == 1 || st == 7 || st == 8 || st == 0x10 || st == 0x14) ? 80 : 152;
    pHelpWnd->super.pane.x0 = x0;
    pHelpWnd->super.pane.y0 = 120;
    pHelpWnd->super.pane.x1 = x0 + 335;
    pHelpWnd->super.pane.y1 = 375;

    WndA2_CallChildrenProc3(&pHelpWnd->super, ASCEND_WM_DEACTIVATE, 0, 0);
    int i;
    for (i = 0; i != 10; ++i)
    {
        pHelpWnd->helpItems[i].hasNextPage = FALSE;
    }
    memset(pHelpWnd->buffer, 0, 3000);
    pHelpWnd->super.shapeCacheIndex = GwshareShpCacheIndexes[0x25];
    pHelpWnd->super.frameShapeNumber = 0;

    AddTextItem(pHelpWnd, st ? "Hotkeys" : "Global Hotkeys", &WinMgr.LargeFont, 0, 15, 2);

    char s1[512], s2[512];
    int y1 = 0;
    bool hasDebugInfo = false;
    char* p1 = s1;
    char* p2 = s2;
    for (i = 0; HelpGuide[st][i].key != NULL; ++i)
    {
        if (HelpGuide[st][i].key == DEBUG)
        {
            hasDebugInfo = true;
            y1 = 35 + GetMultiLineTextHeight(&WinMgr.SmallFont, s1);
            if (CheatMode == TRUE)
            {
                p1 = stpcpy(p1, "\n\n");
                p2 = stpcpy(p2, "\n\n");
                continue;
            }
            else
            {
                break;
            }
        }
        p1 += sprintf(p1, i == 0 ? "%s" : "\n%s", HelpGuide[st][i].key);
        p2 += sprintf(p2, i == 0 ? "%s" : "\n%s", HelpGuide[st][i].desc);
    }
    if (!y1)
    {
        y1 = 35 + GetMultiLineTextHeight(&WinMgr.SmallFont, s1);
    }
    int w1 = GetMultiLineTextWidth(&WinMgr.SmallFont, s1);
    int w2 = GetMultiLineTextWidth(&WinMgr.SmallFont, s2);
    int x1 = (325 - (w1 + 10 + w2)) / 2;

    char* p = pHelpWnd->buffer;
    p1 = stpcpy(p, s1);
    AddTextItem(pHelpWnd, p, &WinMgr.SmallFont, x1, 35, 0);
    p = p1 + 1;
    p1 = stpcpy(p, s2);
    AddTextItem(pHelpWnd, p, &WinMgr.SmallFont, x1 + w1 + 10, 35, 0);

    if (hasDebugInfo)
    {
        if (CheatMode)
        {
            p = p1 + 1;
            p1 = stpcpy(p, "File NOUGAT.LF found, cheats/debug enabled:");
            AddTextItem(pHelpWnd, p, &WinMgr.SmallFont, 0, y1 + 10, 2);
        }
        else if (st == 0)
        {
            p = p1 + 1;
            p1 = stpcpy(p, "Create empty file NOUGAT.LF for cheats/debug");
            AddTextItem(pHelpWnd, p, &WinMgr.SmallFont, 0, y1 + 10, 2);
        }
    }

    P_WndA2 btn = (*pHelpWnd->super.children)[0];
    strcpy(btn->name, "OK");
    btn->sendParam1 = 0;
    WinMgr_DispatchMessageProc3(&WinMgr, btn->hwnd, 0xE, 0, 0);
    WinMgr_ShowModalWindow(pWinMgr, &pHelpWnd->super, 0);
}

static const char* FnButtonNames[5] = {"STATUS", "FLEET", "RESEARCH", "SPECIALAB", "RACES"};

void HotkeyBroadcastHook(P_WinMgr pWinMgr, UWORD message, int ascii, int scancode, int a5)
{
    if (!pWinMgr->modal)
    {
        switch (scancode)
        {
            case KSC_F1:
            {
                P_Wnd20HW pHelpWnd = (P_Wnd20HW)WinMgr_FindWnd(pWinMgr, "HELPWINDOW", 0);
                if (pHelpWnd && HelpGuide[pWinMgr->currentState])
                {
                    ShowHotkeyHelpMem(pWinMgr, pHelpWnd, pWinMgr->currentState);
                }
                return;
            }
            case KSC_F2:
            case KSC_F3:
            case KSC_F4:
            case KSC_F5:
            case KSC_F6:
                if (pWinMgr->currentState == ASCEND_WINMGR_STATE_MAIN_SCREEN)
                {
                    P_WndA2 b = WinMgr_FindWnd(pWinMgr, (char*)FnButtonNames[scancode - 0x3C], 0);
                    if (b && b->shown == TRUE)
                    {
                        b->pProcs->proc3_msg(b, 4, (b->pane.x0 + b->pane.x1) / 2, (b->pane.y0 + b->pane.y1) / 2);
                    }
                    return;
                }
                break;
        }
    }
    WinMgr_BroadcastMessage(pWinMgr, message, ascii, scancode, a5);
}

void PatchHotkeys::Hook(int stage)
{
    switch (stage)
    {
        case 0:
            WriteMemory(0x000549C2 + 1, HotkeyBroadcastHook, false);
            break;
        case 1:
            break;
    }
}

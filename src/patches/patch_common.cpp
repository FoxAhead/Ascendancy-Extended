#include "patches/patch_common.h"
#include "game/types.h"
#include "game/vars.h"
#include "game/functions.h"
#include "game/runtime.h"
#include "utils.h"
#include "version.h"

REGISTER_PATCH(PatchCommon)

static void Patch_FasterStart()
{
    // Faster start
    //   - no delay
    WriteMemory(0x0001C458, NULL, false, OP_NOP, OP_NOP);
    //   - no logo (112: VFX_GIF_draw)
    WriteMemory(0x0001C6F5, NULL, false, OP_NOP, OP_NOP, OP_NOP, OP_NOP, OP_NOP);
    //   - no fade to black (220: VFX_window_fade)
    WriteMemory(0x0001CB58, NULL, false, OP_NOP, OP_NOP, OP_NOP, OP_NOP, OP_NOP);
    //   - 221: FadeEffect = 0;
    WriteMemory(0x0001CB6E + 1, NULL, false, 0, 0, 0, 0);
    //   - 224: GSystem_PaletteSet(&GSystem, 0, 0x100u, black_palette);
    WriteMemory(0x0001CBA8, NULL, false, OP_NOP, OP_NOP, OP_NOP, OP_NOP, OP_NOP);
}

static const char s[] = "https://github.com/FoxAhead/Ascendancy-Extended";

static void Patch_OnEnterStateDrawOpeningGif(WORD state, P_WinState pWinState)
{
    OnEnterStateDrawOpeningGif(state, pWinState);
    WinMgr.SmallFont.pane = GSystem.pane;
    // Font_DrawFormattedText(&WinMgr.SmallFont, 320, 181, s, ASCEND_TF_HALIGN_CENTER_X, 0x43, 0xFF, 0);
    Font_DrawFormattedText(&WinMgr.SmallFont, 152, 181, s, 0, 0x43, 0xFF, 0);
    char s2[] = VERSION_STRING;
    Font_DrawFormattedText(&WinMgr.SmallFont, 484, 181, s2, ASCEND_TF_HALIGN_RIGHT_X, 0x43, 0xFF, 0);
}

void PatchCommon::Hook(int stage)
{
    switch (stage)
    {
        case 0:
            Patch_FasterStart();
            break;
        case 1:
            WinMgr_SetOnEnterStateFunc(&WinMgr, ASCEND_WINMGR_STATE_OPENING, Patch_OnEnterStateDrawOpeningGif);
            break;
    }
}

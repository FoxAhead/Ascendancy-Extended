#pragma once
#include "game/types.h"

extern "C"
{
    void attach();
    void __stdcall patch2_ex(PANE* pPane, P_Planet pPlanet, int color);
    void __stdcall patch4_ex(PANE* pPane, UBYTE planetItemIndex);
}

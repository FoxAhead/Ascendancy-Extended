#include "patches/registry.h"
#include "patches/patch.h"
#include "game/runtime.h"

Patch* Registry::patches[Registry::MAX_PATCHES] = {0};
int Registry::count = 0;

void Registry::Register(Patch* patch)
{
    if (count < MAX_PATCHES)
    {
        patches[count++] = patch;
        // printf("Register %X\n", patch);
    }
}

int Registry::GetCount()
{
    return count;
}

Patch* Registry::GetPatch(int index)
{
    return patches[index];
}

void Registry::CallHook(int stage)
{
    for (int i = 0; i < GetCount(); ++i)
    {
        Patch* patch = GetPatch(i);
        patch->Hook(stage);
        // printf("CallHook(%d) %X\n", stage, patch);
    }
}

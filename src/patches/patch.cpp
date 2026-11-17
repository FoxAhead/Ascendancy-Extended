#include "patches/patch.h"
#include "patches/registry.h"

Patch::Patch()
{
    Registry::Register(this);
}

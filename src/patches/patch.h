#pragma once
#include "game/types.h"

#define REGISTER_PATCH(ClassName) \
    static ClassName g_##ClassName;

class Patch
{
  public:
    T_Procs procs;
    Patch();
    virtual void Hook(int stage) = 0;
    virtual ~Patch() {};
};

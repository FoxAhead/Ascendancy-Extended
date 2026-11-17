#pragma once
#include "patches/patch.h"

class PatchPlanet : public Patch
{
  public:
    virtual void Hook(int stage);
};

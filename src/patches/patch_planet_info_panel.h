#pragma once
#include "patches/patch.h"

class PatchPlanetInfoPanel : public Patch
{
  public:
    virtual void Hook(int stage);
};

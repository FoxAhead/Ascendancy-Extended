#pragma once

class Patch;

class Registry
{
  private:
    static const int MAX_PATCHES = 64;
    static Patch* patches[MAX_PATCHES];
    static int count;

  public:
    static void Register(Patch* patch);
    static int GetCount();
    static Patch* GetPatch(int index);
    static void CallHook(int stage);
};

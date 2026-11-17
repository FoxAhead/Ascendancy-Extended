#ifndef GAME_VARS_H
#define GAME_VARS_H

#include "game/types.h"

extern "C"
{
  extern T_Procs Wnd04BM_Procs;
  extern T_Procs Wnd02COSW_Procs;
  extern T_Procs Wnd10LB_Procs;
  extern T_Procs WndA2_Procs;
  extern T_Procs Wnd29GSW_Procs;
  extern T_Procs Wnd07PW_Procs;
  extern T_Procs Wnd14PSW_Procs;
  extern T_Procs Wnd09RW_Procs;
  extern BYTE LineIndexFromTo[2][4];
  extern UBYTE PlayerRaceMask;
  extern UWORD AbilityDays[21];
  extern UWORD sunsShpCacheIndex2;
  extern UWORD planetsShpCacheIndex2;
  extern UBYTE SpecialDisplayItems[2];
  extern UWORD sunsShpCacheIndex;
  extern UWORD planetsShpCacheIndex;
  extern MACRO_VFX_BOOL CheatMode;
  extern T_Game Game;
  extern T_Keyboard Keyboard;
  extern T_Mouse Mouse;
  extern T_GSystem GSystem;
  extern UBYTE BigBuffer[94816];
  extern UWORD GwshareShpCacheIndexes[45];
  extern T_PlanetItems PlanetItems;
  extern T_PlanetCfg PlanetCfg;
  extern int PlSquareWndMode;
  extern char *PlanetSizeTexts[5];
  extern char *PlanetTypeTexts[11];
  extern UBYTE PlayerRaceIndex;
  extern T_Research Research;
  extern T_Gizmos Gizmos;
  extern T_WinMgr WinMgr;
  extern int TickCounter;
}

#endif

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "game/types.h"

extern int gHazeNum;
extern int gDX;
extern int gDY;

extern "C"
{
    extern uint32_t gRebaseCode;
    extern uint32_t gRebaseData;

    extern void cseg01_start();
    extern void dseg02_start();
    extern void cseg03_start();
    extern void dseg04_start();
}

#define OP_PUSH 0x68
#define OP_NOP  0x90
#define OP_CALL 0xE8
#define OP_JMP  0xE9
#define OP_MOV  0xBA
#define OP_0F   0x0F
#define OP_JZ   0x84
#define OP_JG   0x8F
#define OP_RET  0xC3

#define ASSERT(cond) AssertLogBreakExit((cond), __FILE__, __LINE__)

// PC Keyboard scan codes.
enum KEYBOARD_SCAN_CODES
{
    KSC_UNASSIGNED   = 0x00,
    KSC_ESCAPE       = 0x01,
    KSC_1            = 0x02,
    KSC_2            = 0x03,
    KSC_3            = 0x04,
    KSC_4            = 0x05,
    KSC_5            = 0x06,
    KSC_6            = 0x07,
    KSC_7            = 0x08,
    KSC_8            = 0x09,
    KSC_9            = 0x0A,
    KSC_0            = 0x0B,
    KSC_MINUS        = 0x0C,  // - on main keyboard
    KSC_EQUALS       = 0x0D,  // = on main keyboard
    KSC_BACK         = 0x0E,  // backspace
    KSC_TAB          = 0x0F,
    KSC_Q            = 0x10,
    KSC_W            = 0x11,
    KSC_E            = 0x12,
    KSC_R            = 0x13,
    KSC_T            = 0x14,
    KSC_Y            = 0x15,
    KSC_U            = 0x16,
    KSC_I            = 0x17,
    KSC_O            = 0x18,
    KSC_P            = 0x19,
    KSC_LBRACKET     = 0x1A,  // square bracket open
    KSC_RBRACKET     = 0x1B,  // square bracket close
    KSC_RETURN       = 0x1C,  // Enter on main keyboard
    KSC_LCONTROL     = 0x1D,
    KSC_A            = 0x1E,
    KSC_S            = 0x1F,
    KSC_D            = 0x20,
    KSC_F            = 0x21,
    KSC_G            = 0x22,
    KSC_H            = 0x23,
    KSC_J            = 0x24,
    KSC_K            = 0x25,
    KSC_L            = 0x26,
    KSC_SEMICOLON    = 0x27,
    KSC_APOSTROPHE   = 0x28,  // quote "'"
    KSC_GRAVE        = 0x29,  // accent "`"
    KSC_LSHIFT       = 0x2A,
    KSC_BACKSLASH    = 0x2B,
    KSC_Z            = 0x2C,
    KSC_X            = 0x2D,
    KSC_C            = 0x2E,
    KSC_V            = 0x2F,
    KSC_B            = 0x30,
    KSC_N            = 0x31,
    KSC_M            = 0x32,
    KSC_COMMA        = 0x33,
    KSC_PERIOD       = 0x34,  // . on main keyboard
    KSC_SLASH        = 0x35,  // / on main keyboard
    KSC_RSHIFT       = 0x36,
    KSC_MULTIPLY     = 0x37,  // * on numeric keypad
    KSC_LALT         = 0x38,  // left Alt
    KSC_SPACE        = 0x39,
    KSC_CAPITAL      = 0x3A,
    KSC_F1           = 0x3B,
    KSC_F2           = 0x3C,
    KSC_F3           = 0x3D,
    KSC_F4           = 0x3E,
    KSC_F5           = 0x3F,
    KSC_F6           = 0x40,
    KSC_F7           = 0x41,
    KSC_F8           = 0x42,
    KSC_F9           = 0x43,
    KSC_F10          = 0x44,
    KSC_NUMLOCK      = 0x45,
    KSC_SCROLL       = 0x46,  // Scroll Lock
    KSC_NUMPAD7      = 0x47,
    KSC_NUMPAD8      = 0x48,
    KSC_NUMPAD9      = 0x49,
    KSC_SUBTRACT     = 0x4A,  // - on numeric keypad
    KSC_NUMPAD4      = 0x4B,
    KSC_NUMPAD5      = 0x4C,
    KSC_NUMPAD6      = 0x4D,
    KSC_ADD          = 0x4E,  // + on numeric keypad
    KSC_NUMPAD1      = 0x4F,
    KSC_NUMPAD2      = 0x50,
    KSC_NUMPAD3      = 0x51,
    KSC_NUMPAD0      = 0x52,
    KSC_DECIMAL      = 0x53,  // . on numeric keypad
    KSC_OEM_102      = 0x56,  // < > | on UK/Germany keyboards
    KSC_F11          = 0x57,
    KSC_F12          = 0x58,
    KSC_F13          = 0x64,  //                     (NEC PC98)
    KSC_F14          = 0x65,  //                     (NEC PC98)
    KSC_F15          = 0x66,  //                     (NEC PC98)
    KSC_KANA         = 0x70,  // (Japanese keyboard)
    KSC_ABNT_C1      = 0x73,  // / ? on Portugese (Brazilian) keyboards
    KSC_CONVERT      = 0x79,  // (Japanese keyboard)
    KSC_NOCONVERT    = 0x7B,  // (Japanese keyboard)
    KSC_YEN          = 0x7D,  // (Japanese keyboard)
    KSC_ABNT_C2      = 0x7E,  // Numpad . on Portugese (Brazilian) keyboards
    KSC_NUMPADEQUALS = 0x8D,  // = on numeric keypad (NEC PC98)
    KSC_PREVTRACK    = 0x90,  // Previous Track (KC_CIRCUMFLEX on Japanese keyboard)
    KSC_AT           = 0x91,  //                     (NEC PC98)
    KSC_COLON        = 0x92,  //                     (NEC PC98)
    KSC_UNDERLINE    = 0x93,  //                     (NEC PC98)
    KSC_KANJI        = 0x94,  // (Japanese keyboard)
    KSC_STOP         = 0x95,  //                     (NEC PC98)
    KSC_AX           = 0x96,  //                     (Japan AX)
    KSC_UNLABELED    = 0x97,  //                        (J3100)
    KSC_NEXTTRACK    = 0x99,  // Next Track
    KSC_NUMPADENTER  = 0x9C,  // Enter on numeric keypad
    KSC_RCONTROL     = 0x9D,
    KSC_MUTE         = 0xA0,  // Mute
    KSC_CALCULATOR   = 0xA1,  // Calculator
    KSC_PLAYPAUSE    = 0xA2,  // Play / Pause
    KSC_MEDIASTOP    = 0xA4,  // Media Stop
    KSC_VOLUMEDOWN   = 0xAE,  // Volume -
    KSC_VOLUMEUP     = 0xB0,  // Volume +
    KSC_WEBHOME      = 0xB2,  // Web home
    KSC_NUMPADCOMMA  = 0xB3,  // , on numeric keypad (NEC PC98)
    KSC_DIVIDE       = 0xB5,  // / on numeric keypad
    KSC_SYSRQ        = 0xB7,
    KSC_RALT         = 0xB8,  // right Alt
    KSC_PAUSE        = 0xC5,  // Pause
    KSC_HOME         = 0xC7,  // Home on arrow keypad
    KSC_UP           = 0xC8,  // UpArrow on arrow keypad
    KSC_PGUP         = 0xC9,  // PgUp on arrow keypad
    KSC_LEFT         = 0xCB,  // LeftArrow on arrow keypad
    KSC_RIGHT        = 0xCD,  // RightArrow on arrow keypad
    KSC_END          = 0xCF,  // End on arrow keypad
    KSC_DOWN         = 0xD0,  // DownArrow on arrow keypad
    KSC_PGDOWN       = 0xD1,  // PgDn on arrow keypad
    KSC_INSERT       = 0xD2,  // Insert on arrow keypad
    KSC_DELETE       = 0xD3,  // Delete on arrow keypad
    KSC_LWIN         = 0xDB,  // Left Windows key
    KSC_RWIN         = 0xDC,  // Right Windows key
    KSC_APPS         = 0xDD,  // AppMenu key
    KSC_POWER        = 0xDE,  // System Power
    KSC_SLEEP        = 0xDF,  // System Sleep
    KSC_WAKE         = 0xE3,  // System Wake
};

void WriteMemoryFunc(uint32_t address, const uint8_t opcodes[], size_t size, void* procAddress, bool isAbs);

#define WriteMemory(address, procAddress, isAbs, ...)                                                          \
    do                                                                                                         \
    {                                                                                                          \
        const uint8_t _ops[] = {0, ##__VA_ARGS__};                                                             \
        WriteMemoryFunc((uint32_t)(address), _ops + 1, sizeof(_ops) - 1, (void*)(procAddress), (bool)(isAbs)); \
    } while (0)

bool InsidePane(PANE* pPane, int x, int y);

void InflatePane(PANE*, int, int);

void InflateRect(RECT*, int, int);

void OffsetRect(RECT* pRect, int, int);

POINT RectCenterPoint(const RECT& rect);

void RectToQuad(const RECT& rect, SCRNVERTEX vlist[4]);

void RectToPane(const RECT& rect, PANE* pPane);

void PaneToQuad(const PANE& pane, SCRNVERTEX vlist[4]);

void DrawTextShaded(P_Font pFont, int xOffset, int yOffset, const char* text, UWORD flags, WORD color);

int DaysText(char* text, int days);

int IsWndName(P_WndA2 pWnd, const char* name);

char* strcat(char* dst, const char* t);

char* stpcpy(char* dst, const char* src);

int GetMultiLineTextWidth(P_Font pFont, const char* text);

int GetMultiLineTextHeight(P_Font pFont, const char* text);

float VectorsDistance(const T_Vector3D* pPosition1, const T_Vector3D* pPosition2);

double LaneDistance(const P_Lane pLane);

void GetGizmoExtraInfo(P_Gizmo pGizmo, char* s1, char* s2);

int Curve(int value);

bool PlanetHasItem(P_Planet pPlanet, int planetItemIndex);

bool IsOneTimePlanetItem(int planetItemIndex);

bool IsFixedCostPlanetItem(int planetItemIndex);

bool IsOrbitalWeaponPlanetItem(int planetItemIndex);

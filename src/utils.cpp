#include <stdint.h>
#include <stddef.h>

#include "utils.h"
#include "game/types.h"
#include "game/functions.h"
#include "game/runtime.h"

int gHazeNum = 0;
int gDX = 0;
int gDY = 0;

void WriteMemoryFunc(uint32_t address, const uint8_t opcodes[], size_t size, void* procAddress, bool isAbs)
{
    uint32_t offset;
    address += (address < 0x00090000) ? gRebaseCode : gRebaseData;
    if (size > 0)
    {
        memcpy((void*)address, opcodes, size);
    }
    if (procAddress)
    {
        offset = (uint32_t)procAddress;
        if (!isAbs)
        {
            offset = offset - address - 4 - size;
        }
        memcpy((void*)(address + size), &offset, 4);
    }
}

bool InsidePane(PANE* pPane, int x, int y)
{
    return (x > pPane->x0 && x < pPane->x1 && y > pPane->y0 && y < pPane->y1);
}

void InflatePane(PANE* pPane, int dx, int dy)
{
    // pPane->x0 -= dx;
    // pPane->x1 += dx;
    // pPane->y0 -= dy;
    // pPane->y1 += dy;
    InflateRect((RECT*)&pPane->x0, dx, dy);
}

void InflateRect(RECT* pRect, int dx, int dy)
{
    pRect->x0 -= dx;
    pRect->x1 += dx;
    pRect->y0 -= dy;
    pRect->y1 += dy;
}

void OffsetRect(RECT* pRect, int dx, int dy)
{
    pRect->x0 += dx;
    pRect->x1 += dx;
    pRect->y0 += dy;
    pRect->y1 += dy;
}

POINT RectCenterPoint(const RECT& rect)
{
    POINT result = {(rect.x0 + rect.x1) / 2, (rect.y0 + rect.y1) / 2};
    return result;
}

void RectToQuad(const RECT& rect, SCRNVERTEX vlist[4])
{
    vlist[0].x = rect.x0;
    vlist[0].y = rect.y0;
    vlist[1].x = rect.x1;
    vlist[1].y = rect.y0;
    vlist[2].x = rect.x1;
    vlist[2].y = rect.y1;
    vlist[3].x = rect.x0;
    vlist[3].y = rect.y1;
}

void RectToPane(const RECT& rect, PANE* pPane)
{
    pPane->x0 = rect.x0;
    pPane->x1 = rect.x1;
    pPane->y0 = rect.y0;
    pPane->y1 = rect.y1;
}

void PaneToQuad(const PANE& pane, SCRNVERTEX vlist[4])
{
    vlist[0].x = pane.x0;
    vlist[0].y = pane.y0;
    vlist[1].x = pane.x1;
    vlist[1].y = pane.y0;
    vlist[2].x = pane.x1;
    vlist[2].y = pane.y1;
    vlist[3].x = pane.x0;
    vlist[3].y = pane.y1;
}

void DrawTextShaded(P_Font pFont, int xOffset, int yOffset, const char* text, UWORD flags, WORD color)
{
    pFont->pane.x0++;
    pFont->pane.y0++;
    // Font_DrawFormattedText(pFont, xOffset + 1, yOffset + 1, text, flags, 0, 0xFF, 0);
    Font_DrawFormattedText(pFont, xOffset, yOffset, text, flags, 0, 0xFF, 0);
    pFont->pane.x0--;
    pFont->pane.y0--;
    Font_DrawFormattedText(pFont, xOffset, yOffset, text, flags, color, 0xFF, 0);
}

int DaysText(char* text, int days)
{
    int r = 0;
    text[0] = '\0';
    if (days > 0 && days != 0xFFFF)
    {
        if (days == 1)
        {
            r = sprintf(text, "%d day", days);
        }
        else
        {
            r = sprintf(text, "%d days", days);
        }
    }
    return r;
}

int IsWndName(P_WndA2 pWnd, const char* name)
{
    return !strncmp(pWnd->name, name, 10);
}

char* strcat(char* dst, const char* t)
{
    char* s;
    s = dst;
    while (*s != 0)
        ++s;
    while ((*s++ = *t++) != '\0');
    return (dst);
}

char* stpcpy(char* dst, const char* src)
{
    while ((*dst = *src++) != '\0')
    {
        dst++;
    }
    return dst;
}

// Calculates the maximum line width in multi-line text
int GetMultiLineTextWidth(P_Font pFont, const char* text)
{
    int maxWidth;            // edi
    char* textCopy;          // esi
    char* currentLineStart;  // ebp
    char* currentPos;        // ebx
    char tempChar;           // al
    int lineWidth;           // eax

    if (!text || !pFont || !pFont->pVfxFont)
    {
        return 0;
    }

    maxWidth = 0;
    textCopy = strdup(text);
    if (!textCopy)
    {
        return 0;
    }

    currentLineStart = textCopy;
    currentPos = textCopy;

    while (1)
    {
        tempChar = *currentPos;

        // Temporarily terminate the string with a null character for measurement
        *currentPos = 0;

        // Call the original function to measure the current line
        lineWidth = Font_GetTextWidth(pFont, currentLineStart);

        // Restore the character
        *currentPos = tempChar;

        if (lineWidth > maxWidth)
        {
            maxWidth = lineWidth;
        }

        // If the end of the string (null character) is reached, exit
        if (!tempChar)
        {
            break;
        }

        // If this is a line break, remember the start of the new line
        if (tempChar == '\n')
        {
            currentLineStart = currentPos + 1;
        }

        ++currentPos;
    }

    free(textCopy);
    return maxWidth;
}

// Calculates the actual height that multi-line text will occupy when rendered
int GetMultiLineTextHeight(P_Font pFont, const char* text)
{
    int lineCount;
    int lineHeight;
    const char* p;

    if (!text || !pFont || !pFont->pVfxFont)
    {
        return 0;
    }

    // Count the number of lines (similar to how Q_Font_DrawFormattedText_sub_2B8A8 does it)
    lineCount = 1;
    for (p = text; *p; ++p)
    {
        if (*p == '\n')
        {
            ++lineCount;
        }
    }

    // Line height = font height + 2 pixels of padding
    lineHeight = Font_GetHeight(pFont) + 2;

    return lineCount * lineHeight;
}

float VectorsDistance(const T_Vector3D* pPosition1, const T_Vector3D* pPosition2)
{
    double dx = (double)pPosition1->x - pPosition2->x;
    double dy = (double)pPosition1->y - pPosition2->y;
    double dz = (double)pPosition1->z - pPosition2->z;
    return (float)sqrt(dx * dx + dy * dy + dz * dz);
}

double LaneDistance(const P_Lane pLane)
{
    return VectorsDistance(&pLane->pStar1->position, &pLane->pStar2->position);
}

void GetGizmoExtraInfo(P_Gizmo pGizmo, char* s1, char* s2)
{
    s1[0] = '\0';
    s2[0] = '\0';
    switch (pGizmo->cat)
    {
        case ASCEND_GIZMO_CAT_WEAPON:
            sprintf(s1, "dmg %d\nrng %d", pGizmo->level, pGizmo->range);
            sprintf(s2, "%d pwr\n%d use", pGizmo->power, pGizmo->numUses);
            break;
        case ASCEND_GIZMO_CAT_SHIELD:
            sprintf(s1, "lvl %d", pGizmo->level);
            sprintf(s2, "%d pwr", pGizmo->power);
            break;
        case ASCEND_GIZMO_CAT_DRIVE:
            sprintf(s1, "spd %d", pGizmo->level);
            sprintf(s2, "%d pwr", pGizmo->power);
            break;
        case ASCEND_GIZMO_CAT_SCANNER:
            sprintf(s1, "rng %d", pGizmo->range);
            sprintf(s2, "%d lvl", pGizmo->level);
            break;
        case ASCEND_GIZMO_CAT_GENERATOR:
            // sprintf(s1, "pwr %d", pGizmo->power);
            sprintf(s2, "%d pwr", pGizmo->power);
            break;
            // case ASCEND_GIZMO_CAT_SPECIAL:
            // if (pGizmo->power)
            // sprintf(s1, "pwr %d", pGizmo->power);
            // break;
    }
}

int Curve(int value)
{
    return (int)pow((double)(value + 1), 0.85);
}

//
//  Planet Items
//

bool PlanetHasItem(P_Planet pPlanet, int planetItemIndex)
{
    if (planetItemIndex == ASCEND_PI_37_LUSH_GROWTH_BOMB)
    {
        return ((pPlanet->flags & ASCEND_PF_01_LUSH_GROWTH_BOMBED) != 0);
    }
    else
    {
        int squareIndex = Planet_GetSquareWithItem(pPlanet, planetItemIndex);
        return (squareIndex != 0xFFFF && (pPlanet->pSquares[squareIndex].flags & ASCEND_SQUARE_FLAGS_COMPLETE));
    }
}

bool IsOneTimePlanetItem(int planetItemIndex)
{
    return (planetItemIndex >= ASCEND_PI_12_HYPERPOWER_PLANT && planetItemIndex <= ASCEND_PI_16_OBSERVATION_INSTALLATION || planetItemIndex == ASCEND_PI_37_LUSH_GROWTH_BOMB);
}

bool IsFixedCostPlanetItem(int planetItemIndex)
{
    return (planetItemIndex != ASCEND_PI_23_SHIP && (planetItemIndex < ASCEND_PI_31_ALIEN_HOSPITALITY || planetItemIndex > ASCEND_PI_35_AUTOMATION));
}

bool IsOrbitalWeaponPlanetItem(int planetItemIndex)
{
    return (planetItemIndex == ASCEND_PI_28_ORBITAL_MISSILE_BASE ||
            planetItemIndex == ASCEND_PI_29_SHORT_RANGE_ORBITAL_WHOPPER ||
            planetItemIndex == ASCEND_PI_30_LONG_RANGE_ORBITAL_WHOPPER ||
            planetItemIndex == ASCEND_PI_17_TRACTOR_BEAM);
}

.386p
.MODEL FLAT

ag1 group cseg01

ag2 group dseg02

cseg01_base EQU 00010000h
dseg02_base EQU 00090000h

make_label MACRO laddr, lname
    ORG laddr - cseg01_base
    PUBLIC lname
    lname:
ENDM

make_var MACRO vaddr, vname
    ORG vaddr - dseg02_base
    PUBLIC vname
    vname:
ENDM

cseg01 SEGMENT PARA PRIVATE USE32 'A1'
    make_label 00010000h, cseg01_start_
    INCLUDE game/functions.inc

    make_label 0001C424h, main_

    make_label 00076D09h, sprintf_
    make_label 00076D64h, operator_delete_
    make_label 00076E30h, sqrt_
    make_label 00076EC0h, memset_
    make_label 0007744Eh, fclose_
    make_label 000775DAh, cos_
    make_label 000775E6h, sin_
    make_label 00077F5Eh, fopen_
    make_label 00078097h, printf_
    make_label 0007812Ch, getch_
    make_label 000783B4h, start_
    make_label 000786EAh, fprintf_
    make_label 00078734h, malloc_
    make_label 0007882Fh, free_
    make_label 0007893Bh, strdup_
    make_label 000789BAh, strncmp_
    make_label 00078B20h, pow_
    make_label 00078BDCh, operator_new_
    make_label 00078DC1h, memcpy_
    make_label 00079020h, strcpy_
    make_label 0007904Dh, strlen_
    make_label 0007B8B6h, __InitRtns_
    make_label 00081285h, flushall_

    ORG 00072736h
cseg01 ENDS

dseg02 SEGMENT PARA PRIVATE USE32 'A2'
    make_var 00090000h, dseg02_start_
    make_var 00095A50h, _Wnd04BM_Procs
    make_var 00095BB4h, _Wnd02COSW_Procs
    make_var 00095DD8h, _Wnd10LB_Procs
    make_var 00095E20h, _WndA2_Procs
    make_var 00095F6Ch, _Wnd29GSW_Procs
    make_var 00095F84h, _Wnd07PW_Procs
    make_var 00095FB8h, _Wnd14PSW_Procs
    make_var 00096018h, _Wnd09RW_Procs
    make_var 00096864h, _LineIndexFromTo
    make_var 000968DCh, _PlayerRaceMask
    make_var 000968E8h, _AbilityDays
    make_var 00096B2Ah, _sunsShpCacheIndex2
    make_var 00096B2Ch, _planetsShpCacheIndex2
    make_var 00096B6Ch, _SpecialDisplayItems
    make_var 00096B70h, _sunsShpCacheIndex
    make_var 00096B72h, _planetsShpCacheIndex
    make_var 000A0CFCh, _CheatMode
    make_var 000A2F6Ch, _Game
    make_var 000D85FCh, _Keyboard
    make_var 000D8628h, _Mouse
    make_var 000D8654h, _GSystem
    make_var 000D8DA0h, _BigBuffer
    make_var 000FFEA0h, _GwshareShpCacheIndexes
    make_var 00103F98h, _PlanetItems
    make_var 001045B4h, _PlanetCfg
    make_var 00104684h, _PlSquareWndMode
    make_var 00104688h, _PlanetSizeTexts
    make_var 0010469Ch, _PlanetTypeTexts
    make_var 00104BEAh, _PlayerRaceIndex
    make_var 00105258h, _Research
    make_var 00107020h, _Gizmos
    make_var 0010AE70h, _WinMgr
    make_var 00132B58h, _TickCounter

    ORG 000A8220h
dseg02 ENDS

EXTERN attach_:PROC

.CODE
    PUBLIC cseg03_start_
    cseg03_start_:
    
    forever:
        int     3h
        jmp     short forever
    
    PUBLIC start
    start PROC
        jmp     short around
;
; signature for DOS4GW to identify OW run-time
;
        db      "WATCOM",0

    around:
        sti
        ; Save ES
        push    es
        ; Make ES flat in order REP MOVSB/MOVSD to work correctly
        mov     eax, ds
        mov     es, eax
        ; Calculate rebase offsets
        mov     eax, offset cseg01_start_
        sub     eax, cseg01_base
        mov     _gRebaseCode, eax
        mov     eax, offset dseg02_start_
        sub     eax, dseg02_base
        mov     _gRebaseData, eax
        ; Set keyboard repeat
        mov     ax, 0305h
        xor     bx, bx
        int     16h
        ; Memory patching
        call    attach_
        ; Reset prefetch after memory patching
        jmp     $+2
        ; Restore ES as start_ expects ES = PSP (stores it into __psp)
        pop     es
        ; Run standart start_ routine
        jmp     start_
    start ENDP

.DATA
    PUBLIC dseg04_start_
    dseg04_start_:

    PUBLIC _gRebaseCode
    _gRebaseCode DD ?
    PUBLIC _gRebaseData
    _gRebaseData DD ?

END

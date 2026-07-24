//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KwString.h
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   1998-10-16
//  Comment     :   String Rounte from Delphi 4.0
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KWSTRING_H_
#define _KWSTRING_H_     1


#ifdef _XP_LINUX_

#include <ctype.h>
#include <string.h>
#include <wchar.h>

#pragma pack(push, 1)

extern UCHAR LowerToUpperTable[256 + 4];
extern UCHAR UpperToLowerTable[256 + 4];

#pragma pack(pop)


inline INT WcsLen(LPCUSHORT Str1)
{
    return wcslen((CONST wchar_t *)Str1);
}

inline LPUSHORT WcsCpy(LPUSHORT Dest, LPCUSHORT Source)
{
    return (LPUSHORT)wcscpy((wchar_t *)Dest, (CONST wchar_t *)Source);
}

inline INT WcsCmp(LPCUSHORT Str1, LPCUSHORT Str2)
{
    return wcscmp((CONST wchar_t *)Str1, (CONST wchar_t *)Str2);
}

inline INT WcsNCmp(LPCUSHORT Str1, LPCUSHORT Str2, unsigned MaxLen)
{
    return wcsncmp((CONST wchar_t *)Str1, (CONST wchar_t *)Str2, MaxLen);
}


inline LPUSHORT WcsCat(LPUSHORT DestStr, LPCUSHORT SrcStr)
{
    return (LPUSHORT)wcscat((wchar_t *)DestStr, (CONST wchar_t *)SrcStr);
}

#else   // not _XP_LINUX_                   

#pragma pack(push, 1)

extern UCHAR LowerToUpperTable[256 + 4];
extern UCHAR UpperToLowerTable[256 + 4];

#pragma pack(pop)

inline INT _fastcall WcsLen(LPCUSHORT Str1)
{
    INT Len;

    __asm cld
    __asm MOV    EDI,[Str1]
    __asm MOV    ECX,0xFFFFFFFF
    __asm XOR    ax, ax
    __asm REPNE  SCASW
    __asm MOV    EAX,0xFFFFFFFE
    __asm SUB    EAX,ECX


    __asm MOV    [Len], EAX

    return Len;
}

inline LPUSHORT _fastcall WcsCpy(LPUSHORT Dest, LPCUSHORT Source)
{
    LPUSHORT RetStr;

    __asm cld
    __asm MOV    ESI,[Source]
    __asm MOV    EDX,[Dest]
    __asm MOV    EDI,ESI
    __asm MOV    ECX,0xFFFFFFFF
    __asm XOR    EAX,EAX
    __asm REPNE  SCASW
    __asm NOT    ECX
    __asm MOV    EDI,EDX
    __asm MOV    EDX,ECX
    __asm MOV    EAX,EDI
    __asm SHR    ECX,1
    __asm REP    MOVSD
    __asm MOV    ECX,EDX
    __asm AND    ECX,1
    __asm REP    MOVSW
    __asm MOV    [RetStr], EAX

    return RetStr;
}

inline INT _fastcall WcsCmp(LPCUSHORT Str1, LPCUSHORT Str2)
{
    INT RetCode;

    __asm cld
    __asm MOV    EDI,[Str1]
    __asm MOV    ESI,[Str2]
    __asm mov    edx,edi
    __asm MOV    ECX, -1
    __asm XOR    EAX,EAX
    __asm REPNE  SCASW
    __asm NOT    ECX
    __asm MOV    EDI,EDX
    __asm XOR    EDX,EDX
    __asm REPE   CMPSW
    __asm MOV    AX, [EDI-2]
    __asm MOV    DX, [ESI-2]
    __asm SUB    EAX, EDX
    __asm mov    [RetCode], EAX

    return RetCode;
}

inline INT _fastcall WcsNCmp(LPCUSHORT Str1, LPCUSHORT Str2, unsigned MaxLen)
{
    INT RetCode;
    
    __asm MOV     EDI,[Str1]
    __asm MOV     ESI,[Str2]
    __asm mov     ECX,[MaxLen]
    __asm mov     EDX,EDI

    __asm MOV     EBX,ECX
    __asm XOR     EAX,EAX
    __asm OR      ECX,ECX
    __asm JE      Label1
    __asm REPNE   SCASW
    __asm SUB     EBX,ECX
    __asm MOV     ECX,EBX
    __asm MOV     EDI,EDX
    __asm XOR     EDX,EDX
    __asm REPE    CMPSW
    __asm MOV     AX, [EDI-2]
    __asm MOV     DX, [ESI-2]
    __asm SUB     EAX,EDX

Label1:        
    __asm mov [RetCode], eax

    return RetCode;
}

inline LPUSHORT _fastcall wStrEnd(LPUSHORT Str1)
{
    LPUSHORT RetStr;

    __asm MOV     ECX, 0xFFFFFFFF
    __asm XOR     EAX, EAX
    __asm MOV     EDI, [Str1]
    __asm REPNE   SCASW
    __asm LEA     EAX, [EDI - 2]
    __asm MOV     [RetStr], EAX

    return RetStr;
}

inline LPUSHORT _fastcall WcsCat(LPUSHORT DestStr, LPCUSHORT SrcStr)
{
    return WcsCpy(wStrEnd(DestStr), SrcStr);
}

#endif  // _XP_LINUX_
#endif  // _KWSTRING_H_


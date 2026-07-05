//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KSChar.h
//  Creater     :   Li Yu Xiong
//  Date        :   2002-12-21 20:30:35
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KSCHAR_H
#define _KSCHAR_H

extern UCHAR LowerToUpperTable[256 + 4];
extern UCHAR UpperToLowerTable[256 + 4];

inline INT ToUpper(CHAR ch)
{
    return (UCHAR)LowerToUpperTable[(UCHAR)ch];
}

inline INT ToLower(CHAR ch)
{
    return (UCHAR)UpperToLowerTable[(UCHAR)ch];
}

#endif // _KSCHAR_H

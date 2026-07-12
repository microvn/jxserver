#include "stdafx.h"
#include "Global.h"
#include "KDesignationList.h"
#include "KDesignation.h"

#define PREFIX_INFO_TAB "DesignationPrefixInfo.tab"
#define POSTFIX_INFO_TAB "DesignationPostfixInfo.tab"

BOOL KDesignationList::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadPrefixInfo();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadPostfixInfo();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KDesignationList::UnInit()
{
    m_PrefixList.clear();
    m_PostfixList.clear();
}

KPrefixInfo* KDesignationList::GetPrefixInfo(int nPrefix)
{
    KPrefixInfo*            pResult = NULL;
    KPREFIX_MAP::iterator   it;

    it = m_PrefixList.find(nPrefix);
    KGLOG_PROCESS_ERROR(it != m_PrefixList.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

KPostfixInfo* KDesignationList::GetPostfixInfo(int nPostfix)
{
    KPostfixInfo*           pResult = NULL;
    KPOSTFIX_MAP::iterator  it;

    it = m_PostfixList.find(nPostfix);
    KGLOG_PROCESS_ERROR(it != m_PostfixList.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

BOOL KDesignationList::LoadPrefixInfo()
{
    BOOL        bResult                 = false;
    int         nRetCode                = false;
    ITabFile*   piTabFile               = NULL;
    int         nLine                   = 0;
    int         nHeight                 = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s", SETTING_DIR, PREFIX_INFO_TAB);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight(); 
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int         nID             = 0;
        int         nAnnounceType   = 0;
        int         nCoolDownID     = 0;
        int         nOwnDuration    = 0;
        int         nBuffID         = 0;
        int         nBuffLevel      = 0;
        int         nType           = 0;
        KPrefixInfo Info;

        nRetCode = piTabFile->GetInteger(nLine, "ID", 0, &nID);
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(nID > 0 && nID <= MAX_DESIGNATION_FIX_ID);

        nRetCode = piTabFile->GetInteger(nLine, "AnnounceType", 1, &nAnnounceType);
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(nAnnounceType > datInvalid && nAnnounceType < datTotal);
        Info.nAnnounceType = nAnnounceType;

        // v2.5 new columns (blank in this leak's data -> default 0 = feature dormant)
        nRetCode = piTabFile->GetInteger(nLine, "CoolDownID", 0, &nCoolDownID);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.dwCoolDownID = (DWORD)nCoolDownID;

        nRetCode = piTabFile->GetInteger(nLine, "OwnDuration", 0, &nOwnDuration);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.nOwnDuration = nOwnDuration;

        nRetCode = piTabFile->GetInteger(nLine, "BuffID", 0, &nBuffID);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.dwBuffID = (DWORD)nBuffID;

        nRetCode = piTabFile->GetInteger(nLine, "BuffLevel", 0, &nBuffLevel);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.nBuffLevel = nBuffLevel;

        nRetCode = piTabFile->GetInteger(nLine, "Type", 0, &nType);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.nType = nType;

        m_PrefixList[nID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_PrefixList.clear();
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", PREFIX_INFO_TAB, nLine);
    }
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KDesignationList::LoadPostfixInfo()
{
    BOOL        bResult                 = false;
    int         nRetCode                = false;
    ITabFile*   piTabFile               = NULL;
    int         nLine                   = 0;
    int         nHeight                 = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s", SETTING_DIR, POSTFIX_INFO_TAB);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight(); 
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int             nID             = 0;
        int             nAnnounceType   = 0;
        int             nCoolDownID     = 0;
        int             nOwnDuration    = 0;
        int             nBuffID         = 0;
        int             nBuffLevel      = 0;
        KPostfixInfo    Info;

        nRetCode = piTabFile->GetInteger(nLine, "ID", 0, &nID);
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(nID > 0 && nID <= MAX_DESIGNATION_FIX_ID);

        nRetCode = piTabFile->GetInteger(nLine, "AnnounceType", 0, &nAnnounceType);
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(nAnnounceType > datInvalid && nAnnounceType < datTotal);
        Info.nAnnounceType = nAnnounceType;

        // v2.5 new columns (blank in this leak's data -> default 0 = feature dormant)
        nRetCode = piTabFile->GetInteger(nLine, "CoolDownID", 0, &nCoolDownID);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.dwCoolDownID = (DWORD)nCoolDownID;

        nRetCode = piTabFile->GetInteger(nLine, "OwnDuration", 0, &nOwnDuration);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.nOwnDuration = nOwnDuration;

        nRetCode = piTabFile->GetInteger(nLine, "BuffID", 0, &nBuffID);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.dwBuffID = (DWORD)nBuffID;

        nRetCode = piTabFile->GetInteger(nLine, "BuffLevel", 0, &nBuffLevel);
        (void)nRetCode; /*[endgame] tolerant*/
        Info.nBuffLevel = nBuffLevel;

        m_PostfixList[nID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_PostfixList.clear();
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", POSTFIX_INFO_TAB, nLine);
    }
    KG_COM_RELEASE(piTabFile);
    return bResult;
}


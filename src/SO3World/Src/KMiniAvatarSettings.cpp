#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KMiniAvatar.h"
#include "KMiniAvatarSettings.h"

//////////////////////////////////////////////////////////////////////////
// KMiniAvatarSettings config loader. Columns pinned from v246
// LoadMiniAvatarInfoLine -- see docs/miniavatar_port/WORKLOG.md [RE-2].
//////////////////////////////////////////////////////////////////////////

#define MINIAVATAR_SUB_DIR  "MiniAvatar"
#define MINIAVATAR_INFO_TAB "MiniAvatarInfo.tab"

BOOL KMiniAvatarSettings::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = LoadMiniAvatarInfo();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KMiniAvatarSettings::UnInit()
{
    m_MiniAvatarInfoMap.clear();
}

BOOL KMiniAvatarSettings::LoadMiniAvatarInfo()
{
    BOOL      bResult   = false;
    int       nRetCode  = false;
    ITabFile* piTabFile = NULL;
    int       nLine     = 0;
    int       nHeight   = 0;
    char      szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, MINIAVATAR_SUB_DIR, MINIAVATAR_INFO_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[MiniAvatar] Unable to open MiniAvatarInfo.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int             nID          = 0;
        int             nFreeAcquire = 0;
        KMiniAvatarInfo Info;

        memset(&Info, 0, sizeof(Info));

        nRetCode = piTabFile->GetInteger(nLine, "ID",          0, &nID);                    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ForceID",     0, (int*)&Info.dwForceID);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "FreeAcquire", 0, &nFreeAcquire);           KGLOG_PROCESS_ERROR(nRetCode);
        Info.dwID         = (DWORD)nID;
        Info.bFreeAcquire = (nFreeAcquire != 0);

        if (m_MiniAvatarInfoMap.find((DWORD)nID) != m_MiniAvatarInfoMap.end())
        {
            KGLogPrintf(KGLOG_DEBUG, "[SO3World][MiniAvatar] table \"%s\" line %d: ID %d already exist!", szFileName, nLine, nID);
            goto Exit0;
        }
        m_MiniAvatarInfoMap[(DWORD)nID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", MINIAVATAR_INFO_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

KMiniAvatarInfo* KMiniAvatarSettings::GetMiniAvatarInfo(DWORD dwID)
{
    KMiniAvatarInfo*             pResult = NULL;
    KMINIAVATAR_INFO_MAP::iterator it     = m_MiniAvatarInfoMap.find(dwID);

    KGLOG_PROCESS_ERROR(it != m_MiniAvatarInfoMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

void KMiniAvatarSettings::AcquireAllFreeMiniAvatar(KMiniAvatar* pBox)
{
    KMINIAVATAR_INFO_MAP::iterator it;

    KG_PROCESS_ERROR(pBox);
    for (it = m_MiniAvatarInfoMap.begin(); it != m_MiniAvatarInfoMap.end(); ++it)
    {
        if (it->second.bFreeAcquire)
            pBox->Acquire(it->second.dwID);
    }
Exit0:
    return;
}

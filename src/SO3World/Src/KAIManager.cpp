////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIManager.cpp 
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : 管理各种AI类型数据
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "KAIAction.h"
#include "KAIManager.h"
#include "KSO3World.h"
#include "KAILogic.h"

BOOL KAIManager::Init()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    IIniFile*   piIniFile   = NULL;

    RegisterActionFunctions();

#ifdef _SERVER
    KGLogPrintf(KGLOG_DEBUG, "[AI] loading ... ...");

    bRetCode = LoadAITabListFile();
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLogPrintf(KGLOG_DEBUG, "[AI] %u AI loaded !", m_AITable.size());

    memset(m_ActionRunTimeStatisticsArray, 0, sizeof(m_ActionRunTimeStatisticsArray));
    m_NpcAIRunTimeStatisticsMap.clear();

    m_bLogAIRuntimeStat = 0;

    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
    KGLOG_PROCESS_ERROR(piIniFile);

    bRetCode = piIniFile->GetInteger("AI", "LogAIRuntimeStat", 0, &m_bLogAIRuntimeStat);
    //KGLOG_PROCESS_ERROR(bRetCode);
#endif

    bResult = true;
Exit0:
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

BOOL AIRunTimeStatisticsComp(const std::pair<DWORD, KAI_RUNTIME_STATISTICS_NODE>& crLeft, const std::pair<DWORD, KAI_RUNTIME_STATISTICS_NODE>& crRight)
{
    if (crLeft.second.ullTime == crRight.second.ullTime)
        return crLeft.second.uCount < crRight.second.uCount;

    return crLeft.second.ullTime > crRight.second.ullTime;
}

BOOL KAIManager::LogAIRuntimeStat()
{
    BOOL        bResult         = false;
    time_t      nTime           = g_pSO3World->m_nCurrentTime;
    FILE*       pNpcStatFile    = NULL;
    FILE*       pActionStatFile = NULL;
    char        szFileName[_MAX_PATH];
    struct tm   tmNow; 
    std::map<DWORD, KAI_RUNTIME_STATISTICS_NODE>::iterator  it;
    std::map<DWORD, KAI_RUNTIME_STATISTICS_NODE>::iterator  itEnd;

    localtime_r(&nTime, &tmNow);

    if (!pNpcStatFile)
    {
        snprintf(
            szFileName, sizeof(szFileName), 
            "AIRuntimeStat/%d%2.2d%2.2d-%2.2d%2.2d%2.2d_Npc.tab",
            tmNow.tm_year + 1900,
            tmNow.tm_mon + 1,
            tmNow.tm_mday,
            tmNow.tm_hour,
            tmNow.tm_min,
            tmNow.tm_sec
        );
        szFileName[sizeof(szFileName) - 1] = '\0';

        KG_mkdir("AIRuntimeStat");

        pNpcStatFile = fopen(szFileName, "w");
        KG_PROCESS_ERROR(pNpcStatFile);

        fprintf(pNpcStatFile, "NpcTemp\tTime\tCount\t\n");
    }

    for (it = m_NpcAIRunTimeStatisticsMap.begin(), itEnd = m_NpcAIRunTimeStatisticsMap.end(); it != itEnd; ++it)
        fprintf(pNpcStatFile, "%lu\t%llu\t%u\n", it->first, it->second.ullTime, it->second.uCount);

    if (!pActionStatFile)
    {
        snprintf(
            szFileName, sizeof(szFileName), 
            "AIRuntimeStat/%d%2.2d%2.2d-%2.2d%2.2d%2.2d_Action.tab",
            tmNow.tm_year + 1900,
            tmNow.tm_mon + 1,
            tmNow.tm_mday,
            tmNow.tm_hour,
            tmNow.tm_min,
            tmNow.tm_sec
        );
        szFileName[sizeof(szFileName) - 1] = '\0';

        KG_mkdir("AIRuntimeStat");

        pActionStatFile = fopen(szFileName, "w");
        KG_PROCESS_ERROR(pActionStatFile);

        fprintf(pActionStatFile, "Action\tTime\tCount\t\n");
    }

    for (int i = 0; i < sizeof(m_ActionRunTimeStatisticsArray) / sizeof(m_ActionRunTimeStatisticsArray[0]); i++)
        fprintf(pActionStatFile, "%d\t%llu\t%u\n", i, m_ActionRunTimeStatisticsArray[i].ullTime, m_ActionRunTimeStatisticsArray[i].uCount);

    bResult = true;
Exit0:
    if (pNpcStatFile)
    {
        fclose(pNpcStatFile);
        pNpcStatFile = NULL;
    }

    if (pActionStatFile)
    {
        fclose(pActionStatFile);
        pActionStatFile = NULL;
    }

    return bResult;
}

void KAIManager::UnInit()
{
    for (KAI_TABLE::iterator it = m_AITable.begin(); it != m_AITable.end(); ++it)
    {
        if (it->second.pLogic)
        {
            KMemory::Delete(it->second.pLogic);
            it->second.pLogic = NULL;
        }
    }

    m_AITable.clear();

    if (m_bLogAIRuntimeStat)
        LogAIRuntimeStat();
}

BOOL KAIManager::LoadAITabListFile()
{
    BOOL bResult = false;
    int nRetCode = false;
    ITabFile* piAITabFile = NULL;
    char szListFilePath[] = SETTING_DIR"/AITypeList.tab";
    int nHeight = 0;
    piAITabFile = g_OpenTabFile(szListFilePath);
    if (!piAITabFile)
    {
        KGLogPrintf(KGLOG_ERR, "[AI] Unable to open table file \"%s\"\n", szListFilePath);
        goto Exit0;
    }
    nHeight = piAITabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);
    for (int nRow = 2; nRow <= nHeight; ++nRow)
    {
        char szTabFilePath[MAX_PATH];
        nRetCode = piAITabFile->GetString(nRow, "FilePath", "", szTabFilePath, sizeof(szTabFilePath));
        KGLOG_PROCESS_ERROR(nRetCode == 1);
        KGLOG_PROCESS_ERROR(LoadAITabFile(szTabFilePath));
    }
#ifdef _SERVER
    if (!g_pSO3World->m_bFastBootMode)
        for (KAI_TABLE::iterator it = m_AITable.begin(); it != m_AITable.end(); ++it)
            it->second.pLogic = CreateAI(it->first, it->second.dwScriptID);
#endif
    bResult = true;
Exit0:
    if (!bResult) m_AITable.clear();
    KG_COM_RELEASE(piAITabFile);
    return bResult;
}

BOOL KAIManager::LoadAITabFile(char* szFilePath)
{
    BOOL bResult = false;
    int nRetCode = false;
    ITabFile* piAITabFile = NULL;
    int nHeight = 0;
    piAITabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piAITabFile);
    nHeight = piAITabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);
    for (int nRow = 2; nRow <= nHeight; ++nRow)
    {
        DWORD dwAIType = 0;
        DWORD dwScriptID = 0;
        char szScriptPath[MAX_PATH];
        KAIInfo AIInfo;
        nRetCode = piAITabFile->GetInteger(nRow, "AIType", 0, (int*)&dwAIType);
        (void)nRetCode;
        if (dwAIType == 0) continue;
        nRetCode = piAITabFile->GetString(nRow, "ScriptFile", "", szScriptPath, sizeof(szScriptPath));
        (void)nRetCode;
        dwScriptID = g_FileNameHash(szScriptPath);
        KGLOG_PROCESS_ERROR(dwScriptID);
        AIInfo.dwScriptID = dwScriptID;
        AIInfo.pLogic = NULL;
        m_AITable[dwAIType] = AIInfo;
    }
    bResult = true;
Exit0:
    if (!bResult) m_AITable.clear();
    KG_COM_RELEASE(piAITabFile);
    return bResult;
}

BOOL KAIManager::ReloadAILogic(DWORD dwAIType)
{
    BOOL                bResult                 = false;
    int                 nRetCode                = false;
    KAILogic*           pLogic                  = NULL;
    ITabFile*           piAITabFile             = NULL;
    char                szFilePath[]            = SETTING_DIR"/AIType.tab";
    int                 nHeight                 = 0;
    DWORD               dwScriptID              = 0;
    int                 nRow                    = 0;
    DWORD               dwReadAIType             = 0;
    KAI_TABLE::iterator it;
    char                szScriptPath[MAX_PATH];

    piAITabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piAITabFile);

    nHeight = piAITabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    for (nRow = 2; nRow <= nHeight; nRow++)
    {
        nRetCode = piAITabFile->GetInteger(nRow, "AIType", 0, (int*)&dwReadAIType);
        KGLOG_PROCESS_ERROR(nRetCode == 1);

        if (dwReadAIType != dwAIType)
            continue;

        nRetCode = piAITabFile->GetString(nRow, "ScriptFile", "", szScriptPath, sizeof(szScriptPath));
        KGLOG_PROCESS_ERROR(nRetCode == 1);

        break;
    }

    KGLOG_PROCESS_ERROR(nRow <= nHeight);

    nRetCode = g_pSO3World->m_ScriptCenter.ReloadScript(szScriptPath);
    KGLOG_PROCESS_ERROR(nRetCode);

    dwScriptID = g_FileNameHash(szScriptPath);
    KGLOG_PROCESS_ERROR(dwScriptID);

    it = m_AITable.find(dwAIType);
    if (it == m_AITable.end() || it->second.pLogic == NULL)
    {
        m_AITable[dwAIType].dwScriptID   = dwScriptID;
        m_AITable[dwAIType].pLogic       = CreateAI(dwAIType, dwScriptID);
    }
    else
    {
        it->second.dwScriptID = dwScriptID;

        nRetCode = it->second.pLogic->Setup(dwAIType, dwScriptID);
        KGLOG_PROCESS_ERROR(nRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

KAILogic* KAIManager::GetAILogic(DWORD dwAIType)
{
    KAILogic*           pLogic  = NULL;
    KAIInfo*            pInfo   = NULL;
    KAI_TABLE::iterator it      = m_AITable.find(dwAIType);

    KGLOG_PROCESS_ERROR(it != m_AITable.end());

    pInfo = &it->second;

    if (!pInfo->pLogic)
    {
        pInfo->pLogic = CreateAI(dwAIType, pInfo->dwScriptID);
    }

    pLogic = pInfo->pLogic;
Exit0:
    return pLogic;
}

KAI_ACTION_FUNC KAIManager::GetActionFunction(int nKey)
{
    if (nKey > eakInvalid && nKey < eakTotal)
    {
        return m_ActionFunctionTable[nKey];
    }

    return NULL;
}

KAILogic* KAIManager::CreateAI(DWORD dwType, DWORD dwScriptID)
{
    KAILogic* pResult       = NULL;
    BOOL      bRetCode      = false;
    KAILogic* pAI           = NULL;

    pAI = KMemory::New<KAILogic>();
    KGLOG_PROCESS_ERROR(pAI);

    bRetCode = pAI->Setup(dwType, dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pResult = pAI;
Exit0:
    if (!pResult)
    {
        if (pAI)
        {
            KMemory::Delete(pAI);
            pAI = NULL;
        }

        KGLogPrintf(KGLOG_ERR, "[AI] Setup AI failed, AIType: %d", dwType);
    }
    return pResult;
}


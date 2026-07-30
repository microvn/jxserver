#include "stdafx.h"
#include <assert.h>

#include "KActivityMgrServer.h"
#include "KSO3World.h"
#include "KScene.h"
#include "KScriptCenter.h"
#include "KObjectIndex.h"

DEFINE_LUA_CLASS_BEGIN(KActivityMgrServer)
    REGISTER_LUA_FUNC(KActivityMgrServer, SetActivityState)
    REGISTER_LUA_FUNC(KActivityMgrServer, GetActivityState)
    DEFINE_LUA_CLASS_END(KActivityMgrServer)

KActivityMgrServer::KActivityMgrServer()
{
}

KActivityMgrServer::~KActivityMgrServer()
{
}

BOOL KActivityMgrServer::Init()
{
    return LoadActivity();
}

void KActivityMgrServer::UnInit()
{
    m_ActivityTable.clear();
}

BOOL KActivityMgrServer::LoadActivity()
{
    BOOL        bResult = false;
    ITabFile*   pTabFile = NULL;
    char        szFileName[MAX_PATH];
    char        szScriptFile[MAX_PATH];
    int         nHeight = 0;

    snprintf(szFileName, sizeof(szFileName), "%s/%s", SETTING_DIR, "Activity.tab");
    szFileName[sizeof(szFileName) - 1] = '\0';

    pTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(pTabFile);

    nHeight = pTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 2);

    for (int nLine = 3; nLine <= nHeight; ++nLine)
    {
        DWORD dwActivityID = 0;
        KActivityInfo ActivityInfo;

        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "ID", 0, (int*)&dwActivityID));
        KGLOG_PROCESS_ERROR(dwActivityID != 0);

        ActivityInfo.dwID = dwActivityID;
        ActivityInfo.dwScriptID = 0;
        ActivityInfo.eState = asNormalOff;
        ActivityInfo.nLastProcessTime = 0;

        szScriptFile[0] = '\0';
        KGLOG_PROCESS_ERROR(pTabFile->GetString(nLine, "ServerScript", "", szScriptFile, sizeof(szScriptFile)));
        if (szScriptFile[0] != '\0')
        {
            ActivityInfo.dwScriptID = g_FileNameHash(szScriptFile);
        }

        KGLOG_PROCESS_ERROR(m_ActivityTable.insert(std::make_pair(dwActivityID, ActivityInfo)).second);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(pTabFile);
    if (!bResult)
    {
        m_ActivityTable.clear();
    }
    return bResult;
}

BOOL KActivityMgrServer::IsActivityOn(DWORD dwActivityID)
{
    ACTIVITY_TABLE::iterator it = m_ActivityTable.find(dwActivityID);
    if (it == m_ActivityTable.end())
    {
        return false;
    }
    return it->second.eState == asNormalOn || it->second.eState == asRecoverOn;
}

struct KActivityEndNotifySceneFunc
{
    DWORD dwActivityID;
    BOOL operator()(DWORD, KScene* pScene)
    {
        return pScene && pScene->ActivityEndNotify(dwActivityID);
    }
};

BOOL KActivityMgrServer::StartActivity(DWORD dwActivityID, time_t nProcessTime, ACTIVITY_STATE eState)
{
    ACTIVITY_TABLE::iterator it = m_ActivityTable.find(dwActivityID);
    KGLOG_PROCESS_ERROR(eState == asNormalOn || eState == asRecoverOn);
    KGLOG_PROCESS_ERROR(it != m_ActivityTable.end());
    KGLOG_PROCESS_ERROR(it->second.eState != asNormalOn && it->second.eState != asRecoverOn);

    it->second.eState = eState;
    it->second.nLastProcessTime = nProcessTime;
    if (it->second.dwScriptID != 0)
    {
        KGLOG_PROCESS_ERROR(CallScriptFunction(&it->second, "StartActivity"));
    }
    return true;
Exit0:
    return false;
}

BOOL KActivityMgrServer::EndActivity(DWORD dwActivityID, time_t nProcessTime, ACTIVITY_STATE eState)
{
    ACTIVITY_TABLE::iterator it = m_ActivityTable.find(dwActivityID);
    KGLOG_PROCESS_ERROR(eState == asNormalOff || eState == asDelayOff);
    KGLOG_PROCESS_ERROR(it != m_ActivityTable.end());

    it->second.eState = eState;
    it->second.nLastProcessTime = nProcessTime;
    KActivityEndNotifySceneFunc notifyFunc;
    notifyFunc.dwActivityID = dwActivityID;
    g_pSO3World->m_SceneSet.Traverse(notifyFunc);
    if (it->second.dwScriptID != 0)
    {
        KGLOG_PROCESS_ERROR(CallScriptFunction(&it->second, "EndActivity"));
    }
    return true;
Exit0:
    return false;
}

BOOL KActivityMgrServer::CallScriptFunction(KActivityInfo* pActivity, const char* pszFunction)
{
    int nSafeCallIndex = 0;
    BOOL bResult = false;

    assert(pActivity);
    assert(pszFunction);
    KGLOG_PROCESS_ERROR(pActivity->dwScriptID != 0);
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsScriptExist(pActivity->dwScriptID));
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsFuncExist(pActivity->dwScriptID, pszFunction));

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nSafeCallIndex);
    g_pSO3World->m_ScriptCenter.PushValueToStack((int)pActivity->dwID);
    bResult = g_pSO3World->m_ScriptCenter.CallFunction(pActivity->dwScriptID, pszFunction, 0);
    g_pSO3World->m_ScriptCenter.SafeCallEnd(nSafeCallIndex);
    KGLOG_PROCESS_ERROR(bResult);
    return true;
Exit0:
    return false;
}

BOOL KActivityMgrServer::StoreGmAnnouncement(BYTE* pbyMessage, size_t uMessageLen)
{
    int nSafeCallIndex = 0;
    BOOL bResult = false;
    const char* pszScript = "scripts/ActivityClndMgr.lua";
    const char* pszFunction = "StoreGmAnnouncement";

    KGLOG_PROCESS_ERROR(pbyMessage != NULL);
    KGLOG_PROCESS_ERROR(uMessageLen <= 1);
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsScriptExist(pszScript));
    KGLOG_PROCESS_ERROR(g_pSO3World->m_ScriptCenter.IsFuncExist(pszScript, pszFunction));

    g_pSO3World->m_ScriptCenter.SafeCallBegin(&nSafeCallIndex);
    g_pSO3World->m_ScriptCenter.PushValueToStack((int)g_pSO3World->m_nCurrentTime);
    g_pSO3World->m_ScriptCenter.PushValueToStack((const char*)pbyMessage);
    bResult = g_pSO3World->m_ScriptCenter.CallFunction(pszScript, pszFunction, 0);
    g_pSO3World->m_ScriptCenter.SafeCallEnd(nSafeCallIndex);
    KGLOG_PROCESS_ERROR(bResult);
    return true;
Exit0:
    return false;
}

int KActivityMgrServer::LuaSetActivityState(Lua_State* L)
{
    int nTopIndex = Lua_GetTopIndex(L);
    DWORD dwActivityID = 0;
    ACTIVITY_STATE eState = asNormalOff;
    time_t nProcessTime = 0;

    KGLOG_PROCESS_ERROR(nTopIndex == 3);
    dwActivityID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwActivityID != 0);
    eState = (ACTIVITY_STATE)Lua_ValueToNumber(L, 2);
    nProcessTime = (time_t)Lua_ValueToNumber(L, 3);

    if (eState == asNormalOn || eState == asRecoverOn)
    {
        KGLOG_PROCESS_ERROR(StartActivity(dwActivityID, nProcessTime, eState));
    }
    else
    {
        KGLOG_PROCESS_ERROR(EndActivity(dwActivityID, nProcessTime, eState));
    }
    return 0;
Exit0:
    return 0;
}

int KActivityMgrServer::LuaGetActivityState(Lua_State* L)
{
    int nTopIndex = Lua_GetTopIndex(L);
    DWORD dwActivityID = 0;
    ACTIVITY_TABLE::iterator it;

    KGLOG_PROCESS_ERROR(nTopIndex == 1);
    dwActivityID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwActivityID != 0);

    it = m_ActivityTable.find(dwActivityID);
    if (it == m_ActivityTable.end())
    {
        Lua_PushNil(L);
        return 1;
    }

    Lua_NewTable(L);
    Lua_PushString(L, "nState");
    Lua_PushNumber(L, it->second.eState);
    Lua_SetTable(L, -3);
    Lua_PushString(L, "nProcessTime");
    Lua_PushNumber(L, it->second.nLastProcessTime);
    Lua_SetTable(L, -3);
    return 1;
Exit0:
    Lua_PushNil(L);
    return 1;
}

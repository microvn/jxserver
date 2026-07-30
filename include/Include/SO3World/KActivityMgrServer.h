#ifndef _KACTIVITY_MGR_SERVER_H_
#define _KACTIVITY_MGR_SERVER_H_

#include <map>
#include <time.h>

#include "Global.h"
#include "Luna.h"

enum ACTIVITY_STATE
{
    asNormalOff = 0,
    asDelayOff = 1,
    asNormalOn = 2,
    asRecoverOn = 3,
};

class KActivityMgrServer
{
public:
    struct KActivityInfo
    {
        DWORD           dwID;
        DWORD           dwScriptID;
        ACTIVITY_STATE  eState;
        time_t          nLastProcessTime;
    };

    typedef std::map<DWORD, KActivityInfo> ACTIVITY_TABLE;
    typedef char KActivityInfoSizeCheck[(sizeof(KActivityInfo) == 16) ? 1 : -1];

public:
    KActivityMgrServer();
    ~KActivityMgrServer();

    BOOL Init();
    void UnInit();

    BOOL IsActivityOn(DWORD dwActivityID);

    int LuaSetActivityState(Lua_State* L);
    int LuaGetActivityState(Lua_State* L);

    DECLARE_LUA_CLASS(KActivityMgrServer);

private:
    BOOL LoadActivity();
    BOOL StartActivity(DWORD dwActivityID, time_t nProcessTime, ACTIVITY_STATE eState);
    BOOL EndActivity(DWORD dwActivityID, time_t nProcessTime, ACTIVITY_STATE eState);
    BOOL CallScriptFunction(KActivityInfo* pActivity, const char* pszFunction);
    static BOOL StoreGmAnnouncement(BYTE* pbyMessage, size_t uMessageLen);

    ACTIVITY_TABLE m_ActivityTable;
};

#endif // _KACTIVITY_MGR_SERVER_H_

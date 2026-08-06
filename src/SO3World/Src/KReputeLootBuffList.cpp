#include "stdafx.h"
#include "Global.h"
#include "KReputeLootBuffList.h"
BOOL KReputeLootBuffList::Init()
{
    BOOL bResult = false; ITabFile* pTabFile = NULL; char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "ReputeLootBuff.tab");
    szFile[sizeof(szFile) - 1] = '\0'; pTabFile = g_OpenTabFile(szFile);
    KGLOG_PROCESS_ERROR(pTabFile);
    for (int nLine = 2; nLine <= pTabFile->GetHeight(); ++nLine)
    {
        DWORD dwLootBuffID = 0, dwLootReputeID = 0;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "LootBuffID", 0, (int*)&dwLootBuffID));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "LootReputeID", 0, (int*)&dwLootReputeID));
        // Target uses map::operator[]: a repeated LootBuffID overwrites the
        // previous repute value instead of taking the failure path.
        m_ReputeLootBuffList[dwLootBuffID] = dwLootReputeID;
    }
    bResult = true;
Exit0:
    if (pTabFile) pTabFile->Release(); if (!bResult) m_ReputeLootBuffList.clear(); return bResult;
}
BOOL KReputeLootBuffList::UnInit() { m_ReputeLootBuffList.clear(); return true; }
const KREPUTE_LOOT_BUFF_LIST& KReputeLootBuffList::GetList() const { return m_ReputeLootBuffList; }

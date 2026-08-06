#include "stdafx.h"
#include "Global.h"
#include "KReputeLootBuffList.h"
BOOL KReputeLootBuffList::Init()
{
    BOOL bResult = false; ITabFile* pTabFile = NULL; int nHeight = 0; char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "ReputeLootBuff.tab");
    szFile[sizeof(szFile) - 1] = '\0'; pTabFile = g_OpenTabFile(szFile);
    /* Target 083bc987 uses KGLOG_PROCESS_SUCCESS(!piTabFile) - format string
       0845e7d0 is "KGLOG_PROCESS_SUCCESS(...)" and the branch at 083bc9b3 jumps
       to 083bcaeb, which is exactly where `bResult = 1` is written.  A MISSING
       ReputeLootBuff.tab is therefore a successful, empty load in the target,
       not a failure.  The candidate used KGLOG_PROCESS_ERROR, which would abort
       KWorldSettings::Init if the file were ever absent. */
    KGLOG_PROCESS_SUCCESS(!pTabFile);
    // 083bc9cd: nHeight > 0 IS a hard failure (KGLOG_PROCESS_ERROR, jmp to the
    // no-bResult exit at 083bcaf2). It was missing from the candidate.
    nHeight = pTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);
    for (int nLine = 2; nLine <= nHeight; ++nLine)
    {
        DWORD dwLootBuffID = 0, dwLootReputeID = 0;
        /* Inverted policy in the candidate: the target ignores the LootBuffID
           retcode (083bca54 is followed by a load of the VALUE at -0x14, not of
           %eax) and instead fails on a zero value (083bca59 testl/jne); the
           LootReputeID read at 083bcac1 is not checked at all. */
        pTabFile->GetInteger(nLine, "LootBuffID", 0, (int*)&dwLootBuffID);
        KGLOG_PROCESS_ERROR(dwLootBuffID);
        pTabFile->GetInteger(nLine, "LootReputeID", 0, (int*)&dwLootReputeID);
        // Target uses map::operator[]: a repeated LootBuffID overwrites the
        // previous repute value instead of taking the failure path.
        m_ReputeLootBuffList[dwLootBuffID] = dwLootReputeID;
    }
Exit1:
    bResult = true;
Exit0:
    if (!bResult) m_ReputeLootBuffList.clear(); if (pTabFile) pTabFile->Release(); return bResult;
}
BOOL KReputeLootBuffList::UnInit() { m_ReputeLootBuffList.clear(); return true; }
const KREPUTE_LOOT_BUFF_LIST& KReputeLootBuffList::GetList() const { return m_ReputeLootBuffList; }

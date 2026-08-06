#include "stdafx.h"
#include "Global.h"
#include "KPendentOldDataInfoList.h"

BOOL KPendentOldDataInfoList::UnInit()
{
    m_WaistRepresentID2ItemIDMap.clear();
    m_BackRepresentID2ItemIDMap.clear();
    m_FaceRepresentID2ItemIDMap.clear();
    m_DuplicateRepresetIDItemTransMap.clear();
    return true;
}

DWORD KPendentOldDataInfoList::GetOldWaistPendentItemID(int nRepresentID) const
{
    KREPRESENT_ITEM_MAP::const_iterator it = m_WaistRepresentID2ItemIDMap.find(nRepresentID);
    return it == m_WaistRepresentID2ItemIDMap.end() ? 0 : it->second;
}

DWORD KPendentOldDataInfoList::GetOldBackPendentItemID(int nRepresentID) const
{
    KREPRESENT_ITEM_MAP::const_iterator it = m_BackRepresentID2ItemIDMap.find(nRepresentID);
    return it == m_BackRepresentID2ItemIDMap.end() ? 0 : it->second;
}

DWORD KPendentOldDataInfoList::GetOldFacePendentItemID(int nRepresentID) const
{
    KREPRESENT_ITEM_MAP::const_iterator it = m_FaceRepresentID2ItemIDMap.find(nRepresentID);
    return it == m_FaceRepresentID2ItemIDMap.end() ? 0 : it->second;
}

DWORD KPendentOldDataInfoList::GetDuplicateItemID(DWORD dwRepresentID) const
{
    KDUPLICATE_ITEM_MAP::const_iterator it = m_DuplicateRepresetIDItemTransMap.find(dwRepresentID);
    return it == m_DuplicateRepresetIDItemTransMap.end() ? 0 : it->second;
}

BOOL KPendentOldDataInfoList::Init()
{
    BOOL bResult = false;
    ITabFile* pTabFile = NULL;
    /* Declared with the other locals: KGLOG_PROCESS_ERROR expands to goto Exit0,
       and GCC 4.8 -std=gnu++98 rejects a jump that crosses an initialisation. */
    int nCount = 0;
    char szFile[MAX_PATH];
    /* Target 08213165 pushes "settings/item" (.rodata 0840ec01) as the directory
       argument of the "%s/%s" snprintf at 08213186.  The shipped tab lives at
       settings/item/OldPendentRepresentID2ItemID.tab and there is no copy in
       settings/, so the old SETTING_DIR path made g_OpenTabFile return NULL and
       aborted the whole settings load. */
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR "/item",
        "OldPendentRepresentID2ItemID.tab");
    szFile[sizeof(szFile) - 1] = '\0';
    pTabFile = g_OpenTabFile(szFile);
    KGLOG_PROCESS_ERROR(pTabFile);

    /* Target 08213239: nCount = GetHeight() - 1, loop `i < nCount` (08213779),
       tab row = i + 2 (08213274).  Rows 2..GetHeight() inclusive; the previous
       `nLine < GetHeight()` dropped the last of the 269 data rows. */
    nCount = pTabFile->GetHeight() - 1;
    for (int i = 0; i < nCount; ++i)
    {
        int nLine = i + 2;
        int nPendentType = 0;
        int nRepresentID = 0;
        DWORD dwItemID = 0;
        KREPRESENT_ITEM_MAP* pRepresentMap = NULL;
        /* Column names from the callsite operands: "SubType" (082132a3),
           "RepresetID" (0821336d - the missing 'n' is in both the target string
           at 0840ece5 and the shipped header, do not correct it) and "ItemID"
           (082133ef).  The candidate used "PendentType"/"RepresentID". */
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "SubType", 0, &nPendentType));

        // Target checks the SubType domain at 082132df, BEFORE reading the other
        // two columns, and logs "nSubType == estWaistExtend || ..." on failure.
        KGLOG_PROCESS_ERROR(nPendentType == 11 || nPendentType == 14 ||
            nPendentType == 17);

        // Both remaining reads guard the retcode AND the loaded value
        // (082133a9 "nRetCode && nRepresentID > 0", 0821342b "nRetCode && dwItemID > 0").
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "RepresetID", 0, &nRepresentID));
        KGLOG_PROCESS_ERROR(nRepresentID > 0);
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "ItemID", 0, (int*)&dwItemID));
        KGLOG_PROCESS_ERROR(dwItemID > 0);

        if (nPendentType == 11)
            pRepresentMap = &m_WaistRepresentID2ItemIDMap;
        else if (nPendentType == 14)
            pRepresentMap = &m_BackRepresentID2ItemIDMap;
        else if (nPendentType == 17)
            pRepresentMap = &m_FaceRepresentID2ItemIDMap;
        else
        {
            // Target 08213744: KGLogPrintf(KGLOG_ERR, ...) then Exit0.
            KGLogPrintf(KGLOG_ERR,
                "Load '%s' Failed! unexpected nSubType = %d, Line %d",
                "OldPendentRepresentID2ItemID.tab", nPendentType, nLine);
            goto Exit0;
        }

        KGLOG_PROCESS_ERROR(pRepresentMap);
        std::pair<KREPRESENT_ITEM_MAP::iterator, bool> itInsert =
            pRepresentMap->insert(std::make_pair(nRepresentID, dwItemID));
        if (!itInsert.second)
            m_DuplicateRepresetIDItemTransMap.insert(
                std::make_pair(dwItemID, itInsert.first->second));
    }
    bResult = true;
Exit0:
    if (pTabFile) pTabFile->Release();
    if (!bResult) UnInit();
    return bResult;
}

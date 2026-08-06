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
    char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR,
        "OldPendentRepresentID2ItemID.tab");
    szFile[sizeof(szFile) - 1] = '\0';
    pTabFile = g_OpenTabFile(szFile);
    KGLOG_PROCESS_ERROR(pTabFile);

    for (int nLine = 2; nLine < pTabFile->GetHeight(); ++nLine)
    {
        int nPendentType = 0;
        int nRepresentID = 0;
        DWORD dwItemID = 0;
        KREPRESENT_ITEM_MAP* pRepresentMap = NULL;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "PendentType", 0, &nPendentType));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "RepresentID", 0, &nRepresentID));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "ItemID", 0, (int*)&dwItemID));

        if (nPendentType == 11)
            pRepresentMap = &m_WaistRepresentID2ItemIDMap;
        else if (nPendentType == 14)
            pRepresentMap = &m_BackRepresentID2ItemIDMap;
        else if (nPendentType == 17)
            pRepresentMap = &m_FaceRepresentID2ItemIDMap;
        else
            KGLOG_PROCESS_ERROR(false);

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

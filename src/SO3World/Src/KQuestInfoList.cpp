#include "stdafx.h"
#include "KQuestInfoList.h"
#include "Global.h"

#define QUEST_FILE_NAME	"Quests.tab"

BOOL KQuestInfoList::Init(void)
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(LoadQuestListFile());
    KGLOG_PROCESS_ERROR(LoadRandomQuestGroup());
    bResult = true;
Exit0:
    return bResult;
}

void KQuestInfoList::UnInit()
{
// #ifdef _CLIENT
//     m_QuestStringCache.UnInit();
// #endif
}

KQUEST_GROUP_INFO* KQuestInfoList::GetQuestGroupInfo(DWORD dwGroupID)
{
    MAP_QUEST_GROUP::iterator it = m_QuestGroupMap.find(dwGroupID);
    KG_PROCESS_ERROR(it != m_QuestGroupMap.end());
    return &it->second;
Exit0:
    return NULL;
}

int KQuestInfoList::GetQuestIndexInQuestGroupInfo(DWORD dwNpcTemplateID, DWORD dwQuestID)
{
    MAP_NPC_2_RANDOM_QUEST_GROUP::iterator it = m_mapNpc2RandomQuestGroup.find(dwNpcTemplateID);
    KG_PROCESS_ERROR(it != m_mapNpc2RandomQuestGroup.end());
    for (size_t i = 0; i < it->second.size(); ++i)
    {
        for (size_t j = 0; j < it->second.at((unsigned int)i).QuestGroup.size(); ++j)
        {
            if (it->second.at((unsigned int)i).QuestGroup[j] == dwQuestID)
                return (int)j;
        }
    }
Exit0:
    return -1;
}

KQUEST_GROUP_INFO* KQuestInfoList::GetQuestGroupInfo(DWORD dwNpcTemplateID, DWORD dwQuestID)
{
    MAP_NPC_2_RANDOM_QUEST_GROUP::iterator it = m_mapNpc2RandomQuestGroup.find(dwNpcTemplateID);
    KG_PROCESS_ERROR(it != m_mapNpc2RandomQuestGroup.end());
    for (size_t i = 0; i < it->second.size(); ++i)
    {
        if (std::find(it->second.at((unsigned int)i).QuestGroup.begin(), it->second.at((unsigned int)i).QuestGroup.end(), dwQuestID) != it->second[i].QuestGroup.end())
            return &it->second.at((unsigned int)i);
    }
Exit0:
    return NULL;
}

KQuestInfo* KQuestInfoList::GetQuestInfo(DWORD dwQuestID)
{
    KQuestInfo* pResult = NULL;
	MAP_ID_2_QUEST_INFO::iterator it;

    KG_PROCESS_ERROR(dwQuestID > 0);

	it = m_mapID2QuestInfo.find(dwQuestID);
    KGLOG_PROCESS_ERROR(it != m_mapID2QuestInfo.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

int KQuestInfoList::GetNpcQuestString(
    DWORD dwMapID, DWORD dwNpcTemplateID, char* pszBuffer, size_t uBufferSize
)
{
	int                             nResult     = 0;
    int                             nRetCode    = 0;
    char*                           pszPos      = pszBuffer;
    size_t                          uLeftSize   = uBufferSize;
    MAP_NPC_2_QUEST_INFO::iterator  it;
    MAP_NPC_2_QUEST_INFO::iterator  itUpper;

	assert(pszBuffer);

    it = m_mapNpc2QuestInfo.lower_bound(dwNpcTemplateID);
    itUpper = m_mapNpc2QuestInfo.upper_bound(dwNpcTemplateID);
	for (;it != itUpper; ++it)
	{
		nRetCode = (int)snprintf(pszPos, uLeftSize, "<Q%lu>", it->second);
        KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)uLeftSize);

        uLeftSize -= nRetCode;
        pszPos    += nRetCode;
	}

    nResult = (int)(uBufferSize - uLeftSize);
Exit0:
	return nResult;
}

BOOL KQuestInfoList::GetNpcQuest(DWORD dwNpcTemplateID, std::vector<DWORD>* pvQuestID)
{
	BOOL bResult = false;
    MAP_NPC_2_QUEST_INFO::iterator it;
    MAP_NPC_2_QUEST_INFO::iterator itUpper;

    assert(pvQuestID);

	pvQuestID->clear();

    it = m_mapNpc2QuestInfo.lower_bound(dwNpcTemplateID);
    itUpper = m_mapNpc2QuestInfo.upper_bound(dwNpcTemplateID);
	for (;it != itUpper; ++it)
	{
		pvQuestID->push_back(it->second);
	}

    bResult = true;
Exit0:
	return bResult;
}

BOOL KQuestInfoList::IsNpcQuestExist(DWORD dwNpcTemplateID)
{
    BOOL                           bResult = false;
    MAP_NPC_2_QUEST_INFO::iterator ItLower;
    MAP_NPC_2_QUEST_INFO::iterator ItUpper;

    ItLower = m_mapNpc2QuestInfo.lower_bound(dwNpcTemplateID);
    ItUpper = m_mapNpc2QuestInfo.upper_bound(dwNpcTemplateID);
    KG_PROCESS_ERROR(ItLower != ItUpper);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KQuestInfoList::LoadQuestInfo(ITabFile* piTabFile, int nIndex, KQuestInfo* pQuestInfo)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nTabData = 0;
    char szColName[64];
    char szScriptName[MAX_PATH];

	bRetCode = piTabFile->GetInteger(nIndex, "QuestID", m_DefaultQuestInfo.dwQuestID, (int*)&(pQuestInfo->dwQuestID));
    KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(pQuestInfo->dwQuestID < MAX_QUEST_COUNT);

    bRetCode = piTabFile->GetInteger(nIndex, "Class", m_DefaultQuestInfo.dwClassID, (int *)&(pQuestInfo->dwClassID));
    KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
	bRetCode = piTabFile->GetString(nIndex, "QuestName", m_DefaultQuestInfo.szQuestName, pQuestInfo->szQuestName, QUEST_NAME_LEN);
	KGLOG_PROCESS_ERROR(bRetCode);
#endif

	bRetCode = piTabFile->GetInteger(nIndex, "CanShare", m_DefaultQuestInfo.bShare, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bShare = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "CanRepeat", m_DefaultQuestInfo.bRepeat, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bRepeat = (bool)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "Activity", m_DefaultQuestInfo.bActivity, &nTabData);
    (void)bRetCode;
    pQuestInfo->bActivity = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "CanAssist", m_DefaultQuestInfo.bAssist, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bAssist = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "NeedAccept", m_DefaultQuestInfo.bAccept, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bAccept = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "IsEscortQuest", m_DefaultQuestInfo.bEscortQuest, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bEscortQuest = (bool)nTabData;

    if (pQuestInfo->bEscortQuest && pQuestInfo->bShare)
    {
        KGLogPrintf(KGLOG_WARNING, "Escort quest can't be share! Quest ID : %lu \n", pQuestInfo->dwQuestID);
    }

    bRetCode = piTabFile->GetInteger(nIndex, "QuestLevel", m_DefaultQuestInfo.byLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartMapID", m_DefaultQuestInfo.byStartMapID, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byStartMapID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartNpcTemplateID",
        m_DefaultQuestInfo.dwStartNpcTemplateID, (int*)&(pQuestInfo->dwStartNpcTemplateID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartDoodadTemplateID",
        m_DefaultQuestInfo.dwStartDoodadTemplateID, (int*)&(pQuestInfo->dwStartDoodadTemplateID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemType", m_DefaultQuestInfo.byStartItemType, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byStartItemType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemIndex", m_DefaultQuestInfo.wStartItemIndex, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->wStartItemIndex = (WORD)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinLevel", m_DefaultQuestInfo.byMinLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMinLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MaxLevel", m_DefaultQuestInfo.byMaxLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMaxLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireGender", m_DefaultQuestInfo.byRequireGender, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRequireGender = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireRoleType", m_DefaultQuestInfo.byRequireRoleType, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRequireRoleType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "CooldownID",
        m_DefaultQuestInfo.dwCoolDownID, (int*)&(pQuestInfo->dwCoolDownID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(nIndex, "PrequestLogic", m_DefaultQuestInfo.bPrequestLogic, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bPrequestLogic = (bool)nTabData;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "PrequestID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
            m_DefaultQuestInfo.dwPrequestID[i], (int*)&(pQuestInfo->dwPrequestID[i])
        );
	    KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = piTabFile->GetInteger(
        nIndex, "SubsequenceID",
		m_DefaultQuestInfo.dwSubsequenceID, (int*)&(pQuestInfo->dwSubsequenceID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartTime",
		m_DefaultQuestInfo.nStartTime, (int*)&(pQuestInfo->nStartTime)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndTime",
		m_DefaultQuestInfo.nEndTime, (int*)&(pQuestInfo->nEndTime)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(nIndex, "RequireSkillID", m_DefaultQuestInfo.wRequireSkillID, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->wRequireSkillID = (WORD)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinSkillLevel", m_DefaultQuestInfo.byMinSkillLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMinSkillLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireLifeSkillID", m_DefaultQuestInfo.byRequireLifeSkillID, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRequireLifeSkillID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinLifeSkillLevel", m_DefaultQuestInfo.byMinLifeSkillLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMinLifeSkillLevel = (BYTE)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireApprentice", m_DefaultQuestInfo.bRequireApprentice, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bRequireApprentice = (bool)nTabData;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "StartRequireItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byStartRequireItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byStartRequireItemType[i] = (BYTE)nTabData;

        sprintf(szColName, "StartRequireItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wStartRequireItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wStartRequireItemIndex[i] = (WORD)nTabData;

        sprintf(szColName, "StartRequireItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wStartRequireItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wStartRequireItemAmount[i] = (WORD)nTabData;
    }

#ifdef _SERVER
    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "OfferItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byOfferItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byOfferItemType[i] = (BYTE)nTabData;

        sprintf(szColName, "OfferItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wOfferItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wOfferItemIndex[i] = (WORD)nTabData;

        sprintf(szColName, "OfferItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wOfferItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wOfferItemAmount[i] = (WORD)nTabData;
    }
#endif

    bRetCode = piTabFile->GetInteger(nIndex, "EndMapID", m_DefaultQuestInfo.byEndMapID, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byEndMapID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndNpcTemplateID",
        m_DefaultQuestInfo.dwEndNpcTemplateID, (int*)&(pQuestInfo->dwEndNpcTemplateID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndDoodadTemplateID",
		m_DefaultQuestInfo.dwEndDoodadTemplateID, (int*)&(pQuestInfo->dwEndDoodadTemplateID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(nIndex, "EndItemType", m_DefaultQuestInfo.byEndItemType, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byEndItemType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "EndItemIndex", m_DefaultQuestInfo.wEndItemIndex, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->wEndItemIndex = (WORD)nTabData;

    bRetCode = piTabFile->GetInteger(
        nIndex, "FinishTime",
		m_DefaultQuestInfo.nFinishTime, (int*)&(pQuestInfo->nFinishTime)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    for (int i = 0; i < QUEST_END_ITEM_COUNT; ++i)
    {
        sprintf(szColName, "EndRequireItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byEndRequireItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byEndRequireItemType[i] = (BYTE)nTabData;

        sprintf(szColName, "EndRequireItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wEndRequireItemIndex[i] = (WORD)nTabData;

        sprintf(szColName, "EndRequireItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wEndRequireItemAmount[i] = (WORD)nTabData;

        sprintf(szColName, "DropDoodadTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
            m_DefaultQuestInfo.dwDropItemDoodadTemplateID[i], (int*)&(pQuestInfo->dwDropItemDoodadTemplateID[i])
        );
        KGLOG_PROCESS_ERROR(bRetCode);

        sprintf(szColName, "DropNpcTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
		    m_DefaultQuestInfo.dwDropItemNpcTemplateID[i], (int*)&(pQuestInfo->dwDropItemNpcTemplateID[i])
        );
	    KGLOG_PROCESS_ERROR(bRetCode);

#ifdef _SERVER
        sprintf(szColName, "IsDeleteEndRequireItem%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bIsDeleteEndRequireItem[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->bIsDeleteEndRequireItem[i] = (bool)nTabData;
        sprintf(szColName, "IsCancelDeleteRequireItem%d", i + 1);
        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bCancelDeleteRequireItem[i], &nTabData);
        (void)bRetCode;
        pQuestInfo->bCancelDeleteRequireItem[i] = (bool)nTabData;

        sprintf(szColName, "DropItemRate%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
		    m_DefaultQuestInfo.dwDropItemRate[i], (int*)&(pQuestInfo->dwDropItemRate[i])
        );
	    KGLOG_PROCESS_ERROR(bRetCode);

        sprintf(szColName, "IsDropForEach%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bDropForEach[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->bDropForEach[i] = (bool)nTabData;
#endif
    }

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "KillNpcTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
		    m_DefaultQuestInfo.dwKillNpcTemplateID[i], (int*)&(pQuestInfo->dwKillNpcTemplateID[i])
        );
	    KGLOG_PROCESS_ERROR(bRetCode);

        sprintf(szColName, "KillNpcAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wKillNpcAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wKillNpcAmount[i] = (WORD)nTabData;
    }

    for (int i = 0; i < QUEST_PARAM_COUNT * 2; ++i)
    {
        sprintf(szColName, "QuestValue%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nQuestValue[i], (int*)&(pQuestInfo->nQuestValue[i]));
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    for (int i = 0; i < QUEST_PARAM_COUNT * 2; ++i)
    {
        sprintf(szColName, "RelationDoodadID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.dwRelationDoodadID[i], (int*)&(pQuestInfo->dwRelationDoodadID[i]));
	    KGLOG_PROCESS_ERROR(bRetCode);
    }

	bRetCode = piTabFile->GetInteger(nIndex, "HidePresent", m_DefaultQuestInfo.bHidePresent, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bHidePresent = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll1", m_DefaultQuestInfo.bPresentAll[0], &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bPresentAll[0] = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll2", m_DefaultQuestInfo.bPresentAll[1], &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bPresentAll[1] = (bool)nTabData;

    for (int i = 0; i < 2; ++i)
    {
        sprintf(szColName, "PresentAccordToForce%d", i + 1);
        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bPresentAccordToForce[i], &nTabData);
        (void)bRetCode;
        pQuestInfo->bPresentAccordToForce[i] = (bool)nTabData;
    }

    KGLOG_PROCESS_ERROR(!pQuestInfo->bPresentAccordToForce[0] || pQuestInfo->bPresentAll[0]);
    KGLOG_PROCESS_ERROR(!pQuestInfo->bPresentAccordToForce[1] || pQuestInfo->bPresentAll[1]);

    for (int i = 0; i < KQUEST_PRESENT_ITEM_COUNT; ++i)
    {
        sprintf(szColName, "PresentItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byPresentItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byPresentItemType[i] = (BYTE)nTabData;

        sprintf(szColName, "PresentItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wPresentItemIndex[i] = (WORD)nTabData;

        sprintf(szColName, "PresentItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wPresentItemAmount[i] = (WORD)nTabData;
    }

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentExp",
		m_DefaultQuestInfo.nPresentExp, (int*)&(pQuestInfo->nPresentExp)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentMoney",
		m_DefaultQuestInfo.nPresentMoney, (int*)&(pQuestInfo->nPresentMoney)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "PresentExp2Money", m_DefaultQuestInfo.nPresentExp2Money, (int*)&pQuestInfo->nPresentExp2Money);
    (void)bRetCode;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "AffectForceIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byAffectForceID[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byAffectForceID[i] = (BYTE)nTabData;

        sprintf(szColName, "AffectValue%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nAffectForceValue[i], (int*)&(pQuestInfo->nAffectForceValue[i]));
	    KGLOG_PROCESS_ERROR(bRetCode);
    }

	bRetCode = piTabFile->GetInteger(nIndex, "RepeatCutPercent", m_DefaultQuestInfo.byRepeatCutPercent, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRepeatCutPercent = (BYTE)nTabData;

    pQuestInfo->dwScriptID = 0;
    bRetCode = piTabFile->GetString(nIndex, "ScriptName", "", szScriptName, MAX_PATH);
	KGLOG_PROCESS_ERROR(bRetCode);
    if (szScriptName[0] != '\0')
    {
        pQuestInfo->dwScriptID = g_FileNameHash(szScriptName);
    }

    bRetCode = piTabFile->GetInteger(nIndex, "RewardSkillID", m_DefaultQuestInfo.wPresentSkill, &nTabData);
	(void)bRetCode;
    pQuestInfo->wPresentSkill = (WORD)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RewardSkillMaxLevel", m_DefaultQuestInfo.wPresentSkillMaxLevel, &nTabData);
    (void)bRetCode;
    pQuestInfo->wPresentSkillMaxLevel = (WORD)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "CostMoney", m_DefaultQuestInfo.nCostMoney, (int*)&(pQuestInfo->nCostMoney));
	KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(pQuestInfo->nCostMoney >= 0);

    bRetCode = piTabFile->GetInteger(nIndex, "RequireSchoolMask", m_DefaultQuestInfo.dwRequireSchoolMask, (int*)&(pQuestInfo->dwRequireSchoolMask));
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "RequireReputeAll", m_DefaultQuestInfo.bRequireReputeAll, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bRequireReputeAll = (bool)nTabData;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "RequireForceID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byRequireForceID[i], &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byRequireForceID[i] = (BYTE)nTabData;

        sprintf(szColName, "RequireReputeLevelMin%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byReputeLevelMin[i], &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byReputeLevelMin[i] = (BYTE)nTabData;

        sprintf(szColName, "RequireReputeLevelMax%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byReputeLevelMax[i], &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byReputeLevelMax[i] = (BYTE)nTabData;
    }

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistThew",
		m_DefaultQuestInfo.nPresentAssistThew, &pQuestInfo->nPresentAssistThew
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistStamina",
		m_DefaultQuestInfo.nPresentAssistStamina, &pQuestInfo->nPresentAssistStamina
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistFriendship",
		m_DefaultQuestInfo.nPresentAssistFriendship, &pQuestInfo->nPresentAssistFriendship
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "RequireCamp", m_DefaultQuestInfo.byRequireCampMask, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRequireCampMask = (BYTE)nTabData;

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentPrestige", m_DefaultQuestInfo.nPresentPrestige, (int*)&(pQuestInfo->nPresentPrestige)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentContribution", m_DefaultQuestInfo.nPresentContribution, (int*)&(pQuestInfo->nPresentContribution)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentTrain", m_DefaultQuestInfo.nPresentTrain, (int*)&(pQuestInfo->nPresentTrain)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "PresentJustice", m_DefaultQuestInfo.nPresentJustice, &pQuestInfo->nPresentJustice);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = piTabFile->GetInteger(nIndex, "PresentExamPrint", m_DefaultQuestInfo.nPresentExamPrint, &pQuestInfo->nPresentExamPrint);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = piTabFile->GetInteger(nIndex, "PresentArenaAward", m_DefaultQuestInfo.nPresentArenaAward, &pQuestInfo->nPresentArenaAward);
    (void)bRetCode;
    bRetCode = piTabFile->GetInteger(nIndex, "PresentActivityAward", m_DefaultQuestInfo.nPresentActivityAward, &pQuestInfo->nPresentActivityAward);
    (void)bRetCode;

#ifdef _SERVER
    bRetCode = piTabFile->GetInteger(
        nIndex, "AchievementID", m_DefaultQuestInfo.nAchievementID, &pQuestInfo->nAchievementID
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "AssistMentorValue", m_DefaultQuestInfo.nAssistMentorValue, &pQuestInfo->nAssistMentorValue);
    KGLOG_PROCESS_ERROR(bRetCode);
#endif

    bRetCode = piTabFile->GetInteger(nIndex, "TeamRequireMode", m_DefaultQuestInfo.byTeamRequireMode, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byTeamRequireMode = (BYTE)nTabData;

    pQuestInfo->nTitlePoint = 0;
    bRetCode = piTabFile->GetInteger(nIndex, "TitlePoint", m_DefaultQuestInfo.nTitlePoint, &pQuestInfo->nTitlePoint);
    (void)bRetCode;
    bRetCode = piTabFile->GetInteger(nIndex, "AddTongDevelopmentPoint", m_DefaultQuestInfo.nAddTongDevelopmentPoint, &pQuestInfo->nAddTongDevelopmentPoint);
    (void)bRetCode;
    bRetCode = piTabFile->GetInteger(nIndex, "AddTongFund", m_DefaultQuestInfo.nAddTongFund, &pQuestInfo->nAddTongFund);
    (void)bRetCode;

	bResult = true;
Exit0:
	return bResult;
}

BOOL KQuestInfoList::LoadQuestFile(char* szFilePath, BOOL bIsLoadDefultInfo)
{
    BOOL bResult = false;
    ITabFile* piTabFile = NULL;
    int nHeight = 0;
    KQuestInfo TempQuestInfo;

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);
    piTabFile->SetErrorLog(false);
    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    if (bIsLoadDefultInfo)
    {
        KGLOG_PROCESS_ERROR(LoadQuestInfo(piTabFile, 2, &m_DefaultQuestInfo));
    }
    else
    {
        for (int nIndex = 2; nIndex <= nHeight; ++nIndex)
        {
            KGLOG_PROCESS_ERROR(LoadQuestInfo(piTabFile, nIndex, &TempQuestInfo));
            KGLOG_PROCESS_ERROR(MakeQuestMap(&TempQuestInfo));
            KGLOG_PROCESS_ERROR(m_mapID2QuestInfo.insert(std::make_pair(TempQuestInfo.dwQuestID, TempQuestInfo)).second);
        }
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KQuestInfoList::LoadQuestListFile()
{
    BOOL bResult = false;
    ITabFile* piTabFile = NULL;
    char szPath[MAX_PATH];
    char szFilePath[MAX_PATH];
    int nHeight = 0;
    int nRetCode = snprintf(szPath, sizeof(szPath), "%s/QuestsList.tab", SETTING_DIR);

    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szPath));
    piTabFile = g_OpenTabFile(szPath);
    KGLOG_PROCESS_ERROR(piTabFile);
    piTabFile->SetErrorLog(false);
    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    memset(&m_DefaultQuestInfo, 0, sizeof(m_DefaultQuestInfo));
    for (int nIndex = 2; nIndex <= nHeight; ++nIndex)
    {
        KGLOG_PROCESS_ERROR(piTabFile->GetString(nIndex, "FilePath", "", szFilePath, sizeof(szFilePath)));
        KGLOG_PROCESS_ERROR(LoadQuestFile(szFilePath, nIndex == 2));
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KQuestInfoList::LoadRandomQuestGroup()
{
    BOOL bResult = false;
    ITabFile* piTabFile = NULL;
    char szPath[MAX_PATH];
    char szColumn[64];
    char szScriptName[MAX_PATH];
    int nHeight = 0;
    int nData = 0;
    int nRetCode = snprintf(szPath, sizeof(szPath), "%s/RandomQuestGroup.tab", SETTING_DIR);
    MAP_NPC_2_RANDOM_QUEST_GROUP::iterator itNpcGroup;
    KQuestInfo* pQuestInfo = NULL;

    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szPath));
    piTabFile = g_OpenTabFile(szPath);
    KGLOG_PROCESS_ERROR(piTabFile);
    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    for (int nIndex = 2; nIndex <= nHeight; ++nIndex)
    {
        KQUEST_GROUP_INFO QuestGroupInfo;
        std::pair<MAP_NPC_2_RANDOM_QUEST_GROUP::iterator, bool> InsRet;
        DWORD dwNpcTemplateID = 0;

        piTabFile->GetInteger(nIndex, "GroupID", 0, (int*)&QuestGroupInfo.dwGroupID);
        if (!QuestGroupInfo.dwGroupID)
            continue;
        piTabFile->GetInteger(nIndex, "NpcTemplateID", 0, (int*)&dwNpcTemplateID);
        if (!dwNpcTemplateID)
            continue;
        piTabFile->GetInteger(nIndex, "MaxCount", 1, &QuestGroupInfo.nMaxCount);
        if (QuestGroupInfo.nMaxCount < 1)
            continue;

        itNpcGroup = m_mapNpc2RandomQuestGroup.find(dwNpcTemplateID);
        if (itNpcGroup == m_mapNpc2RandomQuestGroup.end())
        {
            InsRet = m_mapNpc2RandomQuestGroup.insert(std::make_pair(dwNpcTemplateID, RANDOM_QUEST_GROUP_VECTOR()));
            KGLOG_PROCESS_ERROR(InsRet.second);
            itNpcGroup = InsRet.first;
        }

        QuestGroupInfo.dwScriptID = 0;
        szScriptName[0] = "\0"[0];
        piTabFile->GetString(nIndex, "ScriptName", "", szScriptName, sizeof(szScriptName));
        if (szScriptName[0] != "\0"[0])
            QuestGroupInfo.dwScriptID = g_FileNameHash(szScriptName);

        nData = 0;
        piTabFile->GetInteger(nIndex, "PlayerDiff", 0, &nData);
        QuestGroupInfo.bPlayerDiff = nData;
        nData = 0;
        piTabFile->GetInteger(nIndex, "RandomDiff", 0, &nData);
        QuestGroupInfo.bRandomDiff = nData;
        nData = 0;
        piTabFile->GetInteger(nIndex, "Daily", 0, &nData);
        QuestGroupInfo.bDaily = nData;

        for (int nQuestIndex = 0; nQuestIndex < 32; ++nQuestIndex)
        {
            snprintf(szColumn, sizeof(szColumn), "Quest%d", nQuestIndex + 1);
            nData = 0;
            piTabFile->GetInteger(nIndex, szColumn, 0, &nData);
            if (nData == 0)
                break;

            pQuestInfo = GetQuestInfo((DWORD)nData);
            if (!pQuestInfo)
            {
                KGLogPrintf(KGLOG_DEBUG, "Get QuestInfo (QuestID:%d) failed when read 'RandomQuestGroup.tab' line %d.", nData, nIndex);
                goto Exit0;
            }
            if (pQuestInfo->dwStartNpcTemplateID != dwNpcTemplateID)
            {
                KGLogPrintf(KGLOG_DEBUG, "StartNpcTemplateID:%d of QuestInfo (dwQuestID:%lu) is not equal TemplateID:%d in 'RandomQuestGroup.tab' line %d.", pQuestInfo->dwStartNpcTemplateID, (DWORD)nData, dwNpcTemplateID, nIndex);
                goto Exit0;
            }
            if (pQuestInfo->bRepeat != true)
            {
                KGLogPrintf(KGLOG_DEBUG, "QuestInfo (dwQuestID:%lu) is norepeat not valid in 'RandomQuestGroup.tab' line %d.", (DWORD)nData, nIndex);
                goto Exit0;
            }
            QuestGroupInfo.QuestGroup.push_back((DWORD)nData);
        }

        if (QuestGroupInfo.QuestGroup.empty())
            continue;

        if (QuestGroupInfo.bRandomDiff)
            KGLOG_PROCESS_ERROR(QuestGroupInfo.nMaxCount <= (int)QuestGroupInfo.QuestGroup.size());

        itNpcGroup->second.push_back(QuestGroupInfo);
        m_QuestGroupMap.insert(std::make_pair(QuestGroupInfo.dwGroupID, QuestGroupInfo));
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KQuestInfoList::MakeQuestMap(KQuestInfo* pQuestInfo)
{
    BOOL    bResult     = false;
    DWORD   dwQuestID   = 0;
    DWORD   adwTemplateID[QUEST_END_ITEM_COUNT];

    assert(pQuestInfo);

    dwQuestID = pQuestInfo->dwQuestID;
    KGLOG_PROCESS_ERROR(dwQuestID < MAX_QUEST_COUNT);

    m_mapNpc2QuestInfo.insert(std::make_pair(pQuestInfo->dwStartNpcTemplateID, dwQuestID));

    if (pQuestInfo->dwStartNpcTemplateID != pQuestInfo->dwEndNpcTemplateID)
    {
        m_mapNpc2QuestInfo.insert(std::make_pair(pQuestInfo->dwEndNpcTemplateID, dwQuestID));
    }

#ifdef _SERVER

    // 去重
    memcpy(adwTemplateID, pQuestInfo->dwDropItemNpcTemplateID, sizeof(adwTemplateID));
    for (int i = 0; i < QUEST_END_ITEM_COUNT; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            if (adwTemplateID[i] != 0 && adwTemplateID[i] == adwTemplateID[j])
            {
                adwTemplateID[i] = 0;
                break;
            }
        }
    }

    for (int nDropIndex = 0; nDropIndex < QUEST_END_ITEM_COUNT; ++nDropIndex)
    {
        if (adwTemplateID[nDropIndex])
        {
            m_mapNpcID2QuestID.insert(std::make_pair(adwTemplateID[nDropIndex], dwQuestID));
        }
    }
#endif

    // 去重
    memcpy(adwTemplateID, pQuestInfo->dwDropItemDoodadTemplateID, sizeof(adwTemplateID));
    for (int i = 0; i < QUEST_END_ITEM_COUNT; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            if (adwTemplateID[i] != 0 && adwTemplateID[i] == adwTemplateID[j])
            {
                adwTemplateID[i] = 0;
                break;
            }
        }
    }

    for (int nDropIndex = 0; nDropIndex < QUEST_END_ITEM_COUNT; ++nDropIndex)
    {
        if (adwTemplateID[nDropIndex])
        {
            m_mapDoodadID2QuestID.insert(std::make_pair(adwTemplateID[nDropIndex], dwQuestID));
        }
    }

    for (int nDoodadIndex = 0; nDoodadIndex < QUEST_PARAM_COUNT * 2; ++nDoodadIndex)
    {
        if (pQuestInfo->dwRelationDoodadID[nDoodadIndex])
        {
            m_mapRDoodadID2QuestID.insert(
                std::make_pair(pQuestInfo->dwRelationDoodadID[nDoodadIndex], dwQuestID)
            );
        }
    }

    bResult = true;
Exit0:
    return bResult;
}


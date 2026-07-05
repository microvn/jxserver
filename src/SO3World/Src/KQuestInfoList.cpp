#include "stdafx.h"
#include "KQuestInfoList.h"
#include "Global.h"

#define QUEST_FILE_NAME	"Quests.tab"

BOOL KQuestInfoList::Init(void)
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    int         nHeight     = 0;
    ITabFile*   piTabFile   = NULL;
	char        szFilePath[MAX_PATH];
    std::pair<MAP_ID_2_QUEST_INFO::iterator, bool> InsRet;

    nRetCode = (int)snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, QUEST_FILE_NAME);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFilePath));

	piTabFile = g_OpenTabFile(szFilePath);
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
	KGLOG_PROCESS_ERROR(nHeight > 1);

    memset(&m_DefaultQuestInfo, 0, sizeof(m_DefaultQuestInfo));

	nRetCode = LoadQuestInfo(piTabFile, 2, &m_DefaultQuestInfo);
	KGLOG_PROCESS_ERROR(nRetCode);

	for (int nIndex = 3; nIndex <= nHeight; nIndex++)
	{
		KQuestInfo	TempQuestInfo;

		nRetCode = LoadQuestInfo(piTabFile, nIndex, &TempQuestInfo);
		KGLOG_PROCESS_ERROR(nRetCode);

		nRetCode = MakeQuestMap(&TempQuestInfo);
        KGLOG_PROCESS_ERROR(nRetCode);

        InsRet = m_mapID2QuestInfo.insert(std::make_pair(TempQuestInfo.dwQuestID, TempQuestInfo));
        KGLOG_PROCESS_ERROR(InsRet.second);
	}
    
    bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

void KQuestInfoList::UnInit()
{
// #ifdef _CLIENT
//     m_QuestStringCache.UnInit();
// #endif
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
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(nIndex, "Class", m_DefaultQuestInfo.dwClassID, (int *)&(pQuestInfo->dwClassID));
    (void)bRetCode; /*[endgame] tolerant*/

#ifdef _SERVER
	bRetCode = piTabFile->GetString(nIndex, "QuestName", m_DefaultQuestInfo.szQuestName, pQuestInfo->szQuestName, QUEST_NAME_LEN);
	(void)bRetCode; /*[endgame] tolerant*/
#endif
  
	bRetCode = piTabFile->GetInteger(nIndex, "CanShare", m_DefaultQuestInfo.bShare, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bShare = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "CanRepeat", m_DefaultQuestInfo.bRepeat, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bRepeat = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "CanAssist", m_DefaultQuestInfo.bAssist, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bAssist = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "NeedAccept", m_DefaultQuestInfo.bAccept, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bAccept = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "IsEscortQuest", m_DefaultQuestInfo.bEscortQuest, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bEscortQuest = (bool)nTabData;
    
    if (pQuestInfo->bEscortQuest && pQuestInfo->bShare)
    {
        KGLogPrintf(KGLOG_WARNING, "Escort quest can't be share! Quest ID : %lu \n", pQuestInfo->dwQuestID);
    }
    
    bRetCode = piTabFile->GetInteger(nIndex, "QuestLevel", m_DefaultQuestInfo.byLevel, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartMapID", m_DefaultQuestInfo.byStartMapID, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byStartMapID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartNpcTemplateID",
        m_DefaultQuestInfo.dwStartNpcTemplateID, (int*)&(pQuestInfo->dwStartNpcTemplateID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartDoodadTemplateID",
        m_DefaultQuestInfo.dwStartDoodadTemplateID, (int*)&(pQuestInfo->dwStartDoodadTemplateID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemType", m_DefaultQuestInfo.byStartItemType, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byStartItemType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemIndex", m_DefaultQuestInfo.wStartItemIndex, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->wStartItemIndex = (WORD)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinLevel", m_DefaultQuestInfo.byMinLevel, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byMinLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MaxLevel", m_DefaultQuestInfo.byMaxLevel, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byMaxLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireGender", m_DefaultQuestInfo.byRequireGender, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byRequireGender = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireRoleType", m_DefaultQuestInfo.byRequireRoleType, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byRequireRoleType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "CooldownID",
        m_DefaultQuestInfo.dwCoolDownID, (int*)&(pQuestInfo->dwCoolDownID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, "PrequestLogic", m_DefaultQuestInfo.bPrequestLogic, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bPrequestLogic = (bool)nTabData;
    
    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "PrequestID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';
        
        bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
            m_DefaultQuestInfo.dwPrequestID[i], (int*)&(pQuestInfo->dwPrequestID[i])
        );
	    (void)bRetCode; /*[endgame] tolerant*/
    }
    
    bRetCode = piTabFile->GetInteger(
        nIndex, "SubsequenceID", 
		m_DefaultQuestInfo.dwSubsequenceID, (int*)&(pQuestInfo->dwSubsequenceID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartTime",
		m_DefaultQuestInfo.nStartTime, (int*)&(pQuestInfo->nStartTime)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndTime",
		m_DefaultQuestInfo.nEndTime, (int*)&(pQuestInfo->nEndTime)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, "RequireSkillID", m_DefaultQuestInfo.wRequireSkillID, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->wRequireSkillID = (WORD)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinSkillLevel", m_DefaultQuestInfo.byMinSkillLevel, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byMinSkillLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireLifeSkillID", m_DefaultQuestInfo.byRequireLifeSkillID, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byRequireLifeSkillID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinLifeSkillLevel", m_DefaultQuestInfo.byMinLifeSkillLevel, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byMinLifeSkillLevel = (BYTE)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireApprentice", m_DefaultQuestInfo.bRequireApprentice, &nTabData);
    (void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bRequireApprentice = (bool)nTabData;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "StartRequireItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byStartRequireItemType[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byStartRequireItemType[i] = (BYTE)nTabData;

        sprintf(szColName, "StartRequireItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wStartRequireItemIndex[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wStartRequireItemIndex[i] = (WORD)nTabData;

        sprintf(szColName, "StartRequireItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wStartRequireItemAmount[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wStartRequireItemAmount[i] = (WORD)nTabData;
    }

#ifdef _SERVER
    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "OfferItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byOfferItemType[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byOfferItemType[i] = (BYTE)nTabData;
        
        sprintf(szColName, "OfferItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wOfferItemIndex[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wOfferItemIndex[i] = (WORD)nTabData;
        
        sprintf(szColName, "OfferItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wOfferItemAmount[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wOfferItemAmount[i] = (WORD)nTabData;
    }
#endif

    bRetCode = piTabFile->GetInteger(nIndex, "EndMapID", m_DefaultQuestInfo.byEndMapID, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byEndMapID = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndNpcTemplateID",
        m_DefaultQuestInfo.dwEndNpcTemplateID, (int*)&(pQuestInfo->dwEndNpcTemplateID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndDoodadTemplateID", 
		m_DefaultQuestInfo.dwEndDoodadTemplateID, (int*)&(pQuestInfo->dwEndDoodadTemplateID)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(nIndex, "EndItemType", m_DefaultQuestInfo.byEndItemType, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byEndItemType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "EndItemIndex", m_DefaultQuestInfo.wEndItemIndex, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->wEndItemIndex = (WORD)nTabData;
    
    bRetCode = piTabFile->GetInteger(
        nIndex, "FinishTime", 
		m_DefaultQuestInfo.nFinishTime, (int*)&(pQuestInfo->nFinishTime)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    for (int i = 0; i < QUEST_END_ITEM_COUNT; ++i)
    {
        sprintf(szColName, "EndRequireItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byEndRequireItemType[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byEndRequireItemType[i] = (BYTE)nTabData;
        
        sprintf(szColName, "EndRequireItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemIndex[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wEndRequireItemIndex[i] = (WORD)nTabData;
        
        sprintf(szColName, "EndRequireItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemAmount[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wEndRequireItemAmount[i] = (WORD)nTabData;
        
        sprintf(szColName, "DropDoodadTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
            m_DefaultQuestInfo.dwDropItemDoodadTemplateID[i], (int*)&(pQuestInfo->dwDropItemDoodadTemplateID[i])
        );
        (void)bRetCode; /*[endgame] tolerant*/

#ifdef _SERVER
        sprintf(szColName, "IsDeleteEndRequireItem%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bIsDeleteEndRequireItem[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->bIsDeleteEndRequireItem[i] = (bool)nTabData;
        
        sprintf(szColName, "DropNpcTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
		    m_DefaultQuestInfo.dwDropItemNpcTemplateID[i], (int*)&(pQuestInfo->dwDropItemNpcTemplateID[i])
        );
	    (void)bRetCode; /*[endgame] tolerant*/
        
        sprintf(szColName, "DropItemRate%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(
            nIndex, szColName,
		    m_DefaultQuestInfo.dwDropItemRate[i], (int*)&(pQuestInfo->dwDropItemRate[i])
        );
	    (void)bRetCode; /*[endgame] tolerant*/
        
        sprintf(szColName, "IsDropForEach%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bDropForEach[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
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
	    (void)bRetCode; /*[endgame] tolerant*/
        
        sprintf(szColName, "KillNpcAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wKillNpcAmount[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wKillNpcAmount[i] = (WORD)nTabData;
    }
    
    for (int i = 0; i < QUEST_PARAM_COUNT * 2; ++i)
    {
        sprintf(szColName, "QuestValue%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nQuestValue[i], (int*)&(pQuestInfo->nQuestValue[i]));
        (void)bRetCode; /*[endgame] tolerant*/
    }
    
    for (int i = 0; i < QUEST_PARAM_COUNT * 2; ++i)
    {
        sprintf(szColName, "RelationDoodadID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.dwRelationDoodadID[i], (int*)&(pQuestInfo->dwRelationDoodadID[i]));
	    (void)bRetCode; /*[endgame] tolerant*/
    }

	bRetCode = piTabFile->GetInteger(nIndex, "HidePresent", m_DefaultQuestInfo.bHidePresent, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bHidePresent = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll1", m_DefaultQuestInfo.bPresentAll[0], &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bPresentAll[0] = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll2", m_DefaultQuestInfo.bPresentAll[1], &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bPresentAll[1] = (bool)nTabData;
    
    for (int i = 0; i < QUEST_PARAM_COUNT * 2; ++i)
    {
        sprintf(szColName, "PresentItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byPresentItemType[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byPresentItemType[i] = (BYTE)nTabData;
        
        sprintf(szColName, "PresentItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemIndex[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wPresentItemIndex[i] = (WORD)nTabData;
        
        sprintf(szColName, "PresentItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemAmount[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->wPresentItemAmount[i] = (WORD)nTabData;
    }

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentExp",
		m_DefaultQuestInfo.nPresentExp, (int*)&(pQuestInfo->nPresentExp)
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentMoney",
		m_DefaultQuestInfo.nPresentMoney, (int*)&(pQuestInfo->nPresentMoney)
    );
	(void)bRetCode; /*[endgame] tolerant*/
    
    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "AffectForceIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byAffectForceID[i], &nTabData);
	    (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byAffectForceID[i] = (BYTE)nTabData;
        
        sprintf(szColName, "AffectValue%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nAffectForceValue[i], (int*)&(pQuestInfo->nAffectForceValue[i]));
	    (void)bRetCode; /*[endgame] tolerant*/
    }

	bRetCode = piTabFile->GetInteger(nIndex, "RepeatCutPercent", m_DefaultQuestInfo.byRepeatCutPercent, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byRepeatCutPercent = (BYTE)nTabData;
    
    pQuestInfo->dwScriptID = 0;
    bRetCode = piTabFile->GetString(nIndex, "ScriptName", "", szScriptName, MAX_PATH);
	(void)bRetCode; /*[endgame] tolerant*/
    if (szScriptName[0] != '\0')
    {
        pQuestInfo->dwScriptID = g_FileNameHash(szScriptName);
    }
    
    bRetCode = piTabFile->GetInteger(nIndex, "RewardSkillID", m_DefaultQuestInfo.wPresentSkill, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->wPresentSkill = (WORD)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "CostMoney", m_DefaultQuestInfo.nCostMoney, (int*)&(pQuestInfo->nCostMoney));
	(void)bRetCode; /*[endgame] tolerant*/
    KGLOG_PROCESS_ERROR(pQuestInfo->nCostMoney >= 0);

    bRetCode = piTabFile->GetInteger(nIndex, "RequireSchoolMask", m_DefaultQuestInfo.dwRequireSchoolMask, (int*)&(pQuestInfo->dwRequireSchoolMask));
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(nIndex, "RequireReputeAll", m_DefaultQuestInfo.bRequireReputeAll, &nTabData);
    (void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->bRequireReputeAll = (bool)nTabData;

    for (int i = 0; i < QUEST_PARAM_COUNT; ++i)
    {
        sprintf(szColName, "RequireForceID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byRequireForceID[i], &nTabData);
        (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byRequireForceID[i] = (BYTE)nTabData;

        sprintf(szColName, "RequireReputeLevelMin%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byReputeLevelMin[i], &nTabData);
        (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byReputeLevelMin[i] = (BYTE)nTabData;
        
        sprintf(szColName, "RequireReputeLevelMax%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byReputeLevelMax[i], &nTabData);
        (void)bRetCode; /*[endgame] tolerant*/
        pQuestInfo->byReputeLevelMax[i] = (BYTE)nTabData;
    }

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistThew",
		m_DefaultQuestInfo.nPresentAssistThew, &pQuestInfo->nPresentAssistThew
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistStamina",
		m_DefaultQuestInfo.nPresentAssistStamina, &pQuestInfo->nPresentAssistStamina
    );
	(void)bRetCode; /*[endgame] tolerant*/

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentAssistFriendship",
		m_DefaultQuestInfo.nPresentAssistFriendship, &pQuestInfo->nPresentAssistFriendship
    );
	(void)bRetCode; /*[endgame] tolerant*/
    
    bRetCode = piTabFile->GetInteger(nIndex, "RequireCamp", m_DefaultQuestInfo.byRequireCampMask, &nTabData);
	(void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byRequireCampMask = (BYTE)nTabData;

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentPrestige", m_DefaultQuestInfo.nPresentPrestige, (int*)&(pQuestInfo->nPresentPrestige)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentContribution", m_DefaultQuestInfo.nPresentContribution, (int*)&(pQuestInfo->nPresentContribution)
    );
	(void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(
        nIndex, "PresentTrain", m_DefaultQuestInfo.nPresentTrain, (int*)&(pQuestInfo->nPresentTrain)
    );
	(void)bRetCode; /*[endgame] tolerant*/

#ifdef _SERVER
    bRetCode = piTabFile->GetInteger(
        nIndex, "AchievementID", m_DefaultQuestInfo.nAchievementID, &pQuestInfo->nAchievementID
    );
    (void)bRetCode; /*[endgame] tolerant*/

    bRetCode = piTabFile->GetInteger(nIndex, "AssistMentorValue", m_DefaultQuestInfo.nAssistMentorValue, &pQuestInfo->nAssistMentorValue);
    (void)bRetCode; /*[endgame] tolerant*/
#endif

    bRetCode = piTabFile->GetInteger(nIndex, "TeamRequireMode", m_DefaultQuestInfo.byTeamRequireMode, &nTabData);
    (void)bRetCode; /*[endgame] tolerant*/
    pQuestInfo->byTeamRequireMode = (BYTE)nTabData;

	bResult = true;
Exit0:
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


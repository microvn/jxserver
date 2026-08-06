#include "stdafx.h"
#include "KMapListFile.h"
#include "KSO3World.h"
#include "KScene.h"

BOOL KMapListFile::Init(void)
{
    BOOL        bResult     = false;
	BOOL        bRetCode    = false;
	ITabFile*	piTabFile   = NULL;
	char        szFilePath[MAX_PATH];

	snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, MAP_LIST_FILE_NAME);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

	piTabFile = g_OpenTabFile(szFilePath);
	KGLOG_PROCESS_ERROR(piTabFile);

	for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
	{
        KMapParams  MapParam;
        DWORD       dwMapID = 0;
        int         nMaxMapLevel = 0;
        int         nCopyIndex = 0;
        memset(&MapParam, 0, sizeof(MapParam));

		bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwMapID);
		KGLOG_PROCESS_ERROR(dwMapID <= MAX_MAP_ID);

		bRetCode = piTabFile->GetString(nRowIndex, "Name", "", MapParam.szMapName, sizeof(MapParam.szMapName));
		KGLOG_PROCESS_ERROR(MapParam.szMapName[0] != '\0');

		bRetCode = piTabFile->GetString(nRowIndex, "DisplayName", MapParam.szMapName, MapParam.szDisplayName, sizeof(MapParam.szDisplayName));
		KGLOG_PROCESS_ERROR(MapParam.szDisplayName[0] != '\0');

        bRetCode = piTabFile->GetInteger(nRowIndex, "Type", 1, &MapParam.nType);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AllScenePlayerInFight", 0, &MapParam.bAllScenePlayerInFight);

		bRetCode = piTabFile->GetInteger(nRowIndex, "Broadcast", 1, &MapParam.nBroadcast);

        bRetCode = piTabFile->GetInteger(nRowIndex, "bCanTongWar", 0, &MapParam.bCanTongWar);

        bRetCode = piTabFile->GetInteger(nRowIndex, "bCanPK", 1, &MapParam.bCanPK);

        bRetCode = piTabFile->GetInteger(nRowIndex, "bCanDuel", 1, &MapParam.bCanDuel);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CampType", emctAllProtect, &MapParam.nCampType);

        bRetCode = piTabFile->GetInteger(nRowIndex, "NeedCampBuff", 0, &MapParam.bNeedCampBuff);
        KGLOG_PROCESS_ERROR(MapParam.nCampType > emctInvalid && MapParam.nCampType < emctTotal);

		bRetCode = piTabFile->GetInteger(nRowIndex, "MapDrop", 0, (int*)&MapParam.dwMapDropID);

		bRetCode = piTabFile->GetString(nRowIndex, "ResourcePath", "", MapParam.szResourceFilePath, sizeof(MapParam.szResourceFilePath));

        bRetCode = piTabFile->GetInteger(nRowIndex, "ReviveInSitu", 0, (int*)&MapParam.bReviveInSitu);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxPlayerCount", 0, (int*)&MapParam.nMaxPlayerCount);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BanSkillMask", 0, (int*)&MapParam.dwBanSkillMask);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BanUseItemMask", 0, (int*)&MapParam.dwBanUseItemMask);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BattleRelationMask", 0, (int*)&MapParam.dwBattleRelationMask);

        bRetCode = piTabFile->GetInteger(nRowIndex, "DoNotGoThroughRoof", 0, (int*)&MapParam.bDoNotGoThroughRoof);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "RefreshCycle", 0, (int*)&MapParam.nRefreshCycle);

        bRetCode = piTabFile->GetInteger(nRowIndex, "RefreshOffset", 0, &MapParam.nRefreshOffset);

        bRetCode = piTabFile->GetInteger(nRowIndex, "QuestCountAchID", -1, &MapParam.nQuestCountAchID);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "LimitTimes", 0, &MapParam.nLimitTimes);

        bRetCode = piTabFile->GetInteger(nRowIndex, "FightList", 0, &MapParam.bFightList);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxMapLevel", 0, &nMaxMapLevel);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxLootRange", 1, &MapParam.nMaxLootRange);

        bRetCode = piTabFile->GetInteger(nRowIndex, "IsArenaMap", 0, &MapParam.bIsArenaMap);

        bRetCode = piTabFile->GetInteger(nRowIndex, "NewCampFight", 0, &MapParam.bNewCampFight);
        
        MapParam.nRefreshCycle *= 60;
        MapParam.nRefreshOffset *= 60;
        MapParam.nRefreshOffset += timezone;

		for (nCopyIndex = 0; nCopyIndex <= nMaxMapLevel; ++nCopyIndex)
		{
			MapParam.dwMapID = KScene::MakeMapKey(dwMapID, nCopyIndex);
			m_MapParamTable[MapParam.dwMapID] = MapParam;
		}
	}

    bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

KMapParams*	KMapListFile::GetMapParamByID(DWORD dwMapID)
{
    KMAP_PARAM_TABLE::iterator it = m_MapParamTable.find(dwMapID);

    if (it != m_MapParamTable.end())
    {
        return &it->second;
    }

	return NULL;
}

KMapParams*	KMapListFile::GetMapParamByName(const char cszName[])
{
    int                        nRetCode = 0;
    KMAP_PARAM_TABLE::iterator it       = m_MapParamTable.begin();
    
    while (it != m_MapParamTable.end())
    {
        nRetCode = strcmp(it->second.szMapName, cszName);
        if (nRetCode == 0)
        {
            return &it->second;
        }
        ++it;
    }

    return NULL;
}

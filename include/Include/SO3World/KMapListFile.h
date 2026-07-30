#ifndef _KMAP_LIST_FILE_H_
#define _KMAP_LIST_FILE_H_

#include <cstddef>
#include "Global.h"

struct KMapParams
{
    DWORD   dwMapID;
    char    szMapName[_NAME_LEN];
    char    szDisplayName[_NAME_LEN];
    int     nBroadcast;
    BOOL    bCanTongWar;
    BOOL    bCanPK;
    BOOL    bCanDuel;
    int     nCampType;
    BOOL    bAllScenePlayerInFight;
    DWORD   dwMapDropID;
    int     bReviveInSitu;
    char    szResourceFilePath[MAX_PATH];
    int     nMaxPlayerCount;
    DWORD   dwBanSkillMask;
    DWORD   dwBanUseItemMask;
    DWORD   dwBattleRelationMask;
    BOOL    bDoNotGoThroughRoof;
    int     nType;
    int     nRefreshCycle;
    int     nRefreshOffset;
    int     nQuestCountAchID;
    int     nLimitTimes;
    BOOL    bFightList;
    BOOL    bNeedCampBuff;
    int     nMaxLootRange;
    BOOL    bIsArenaMap;
    BOOL    bNewCampFight;
};

typedef char KMAP_PARAMS_TARGET_SIZE[(sizeof(KMapParams) == 0x1A4) ? 1 : -1];
typedef char KMAP_PARAMS_TARGET_DROP_ID[(offsetof(KMapParams, dwMapDropID) == 0x5C) ? 1 : -1];

typedef std::map<DWORD, KMapParams> KMAP_PARAM_TABLE;

class KMapListFile
{
public:
	BOOL Init();
    void UnInit(){};

	KMapParams*	GetMapParamByID(DWORD dwMapID);
    KMapParams*	GetMapParamByName(const char cszName[]);

    const KMAP_PARAM_TABLE& GetMapList() { return m_MapParamTable; }

private:
	KMAP_PARAM_TABLE m_MapParamTable;
};

#endif	//_KMAP_LIST_FILE_H_

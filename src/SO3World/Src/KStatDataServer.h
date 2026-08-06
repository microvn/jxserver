#ifndef _KSTAT_DATA_SERVER_H_
#define _KSTAT_DATA_SERVER_H_

#ifdef _SERVER

#include "Engine/KMemory.h"
#include "KStatDataDef.h"
#include <list>

class KPlayer;
class KScene;

// Server自己维护的统计数据结构
struct KStatData
{
    KStatData() : m_nValue(0), m_nID(0), m_bInList(false) {}
    char    m_szName[STAT_DATA_NAME_LEN];
    int64_t m_nValue;
    int     m_nID;
    BOOL    m_bInList;
};

class KStatDataServer
{
public:
    KStatDataServer();
    ~KStatDataServer();

    BOOL Init();
    void UnInit();

    void Activate();
    void SendAllStatData();
    int  GetUpdateStatData(KSTAT_DATA_MODIFY* pStatData, int nMaxCount);
    BOOL Update(const char cszName[], int64_t nValue);
    BOOL SetNameID(const char cszName[], int nID);

public:
    void UpdateMoneyStat(KPlayer* pPlayer, int nMoney, const char cszMethod[]);
    void UpdateContributeStat(KPlayer* pPlayer, int nContribute, const char cszMethod[]);
    // --- v2.5 currency/diamond stats, ported 1:1 from target SO3GameServerD ---
    // target KStatDataServer::UpdateJusticeStat       @0x081d2e08
    void UpdateJusticeStat(KPlayer* pPlayer, int nJustice, const char* cszMethod);
    // target KStatDataServer::UpdateExamPrintStat     @0x081d2d28
    void UpdateExamPrintStat(KPlayer* pPlayer, int nExamPrint, const char* cszMethod);
    // target KStatDataServer::UpdateArenaAwardStat    @0x081d2c48
    void UpdateArenaAwardStat(KPlayer* pPlayer, int nArenaAward, const char* cszMethod);
    // target KStatDataServer::UpdateActivityAwardStat @0x081d2b68
    void UpdateActivityAwardStat(KPlayer* pPlayer, int nActivityAward, const char* cszMethod);
    // target KStatDataServer::UpdateDiamondStat       @0x081d1aae
    void UpdateDiamondStat(int nCount, int nItemDetail, const char* cszMethod);
    // target KStatDataServer::UpdateMoneyStat         @0x081d2fc8 -- signature and key
    // format differ from the 2010 overload above (no KPlayer, no map/level in the key).
    // Added as an overload rather than replacing the 2010 one: 8 call sites outside
    // this ticket's allowlist still use the (KPlayer*, int, const char[]) form.
    // See compile-gate.md "UpdateMoneyStat reconciliation".
    void UpdateMoneyStat(int nMoney, const char* cszMethod, const char* cszSubType);
    void UpdateSkillStat(DWORD dwSkillID, DWORD dwSkillLevel);
    void UpdateFormationStat(DWORD dwFormationID, DWORD dwFormationLevel);
    void UpdateAutoFlyStat(int nValue, const char cszMethod[]);
    void UpdateLearnProfessionLevelState(DWORD dwProfessionID, DWORD dwLevel);
    void UpdateLearnProfessionBranchState(DWORD dwProfessionID, DWORD dwBranchID);
    void UpdateRecipeCastState(DWORD dwCraftID, DWORD dwRecipeID);
    void UpdateEnterScene(KScene* pScene, KPlayer* pPlayer);
    void UpdateItemDrop(DWORD dwItemTab, DWORD dwItemIndex);
    void UpdateTrainState(int nTrain);

    void UpdateQuestStat(DWORD dwQuestID, const char cszState[]);
    void UpdateQuestRepeat(DWORD dwQuestID);
    void UpdateAssistQuest(DWORD dwQuestID);
    void UpdateMentorAssistQuest(DWORD dwQuestID);
    void UpdatePVEStat(DWORD dwVictimID, DWORD dwKillerID);

    void UpdateTalkStat(KPlayer* pPlayer, const char* cszMsgType);
    void UpdatePrestigeStat(KPlayer* pPlayer, int nPrestige, const char* cszMethod);
    void UpdateAchievementPointStat(KPlayer* pPlayer, int nPoint);
    void UpdateAchievementStat(int nID);
    void UpdateDesignationStat(BOOL bIsPrefix, int nID);
    void UpdateShopRepairItemStat(KPlayer* pPlayer, BOOL bIsAllRepair);
    void UpdateExpStat(KPlayer* pPlayer, int nAddExp, const char* cszMethod);
    void UpdateClientNormalLogout();
    void UpdateClientErrorLogout();
    void UpdateClientLoginPermit();
    void UpdateClientTimeout();

private:
    struct KStatNameLess
    {
        bool operator()(const char* pszX, const char* pszY) const
        {
            return strcmp(pszX, pszY) < 0;
        }
    };

    typedef KMemory::KAllocator<std::pair<char*, KStatData*> > KDATA_TABLE_ALLOCATOR;
    typedef std::map<char*, KStatData*, KStatNameLess, KDATA_TABLE_ALLOCATOR> KDATA_TABLE;

    KDATA_TABLE     m_DataTable;

    typedef std::list<KStatData*, KMemory::KAllocator<KStatData*> > KDATA_QUEUE;
    KDATA_QUEUE     m_DataList;

    std::set<kstring, std::less<kstring>, KMemory::KAllocator<kstring> > m_Name2IDQueue;
};
#endif // _SERVER

#endif

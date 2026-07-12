#ifndef _KREGRESSION_MANAGER_H_
#define _KREGRESSION_MANAGER_H_

//////////////////////////////////////////////////////////////////////////
// KRegressionManager -- returning-player (回归) reward config singleton.
// Direct KSO3World member (KMentorCache pattern), Init in KSO3World::Init.
// Loads [REGRESSION] from ServerConstListByVer.ini + RegressionReward.tab.
// NEW in v2.5; layout pinned from DWARF -- docs/regression_port/WORKLOG.md.
//////////////////////////////////////////////////////////////////////////

#include <map>

#define REGRESSION_DAILY_MAX_COUNT  8
#define REGRESSION_ITEM_MAX_COUNT   8

// One reward's 8 item slots (KRegressionManager.h, size 0x60).
struct KRewardItem
{
    DWORD   dwItemType[REGRESSION_ITEM_MAX_COUNT];   // +0x00
    int     dwItemIndex[REGRESSION_ITEM_MAX_COUNT];  // +0x20
    int     nItemStackNum[REGRESSION_ITEM_MAX_COUNT];// +0x40
};                                                   // sizeof = 0x60

// Per-(grade,kungfu) reward info (size 0x20).
struct KRewardItemInfo
{
    BOOL    bFreeLimit;             // +0x00
    int     nRegressionDailyCount;  // +0x04
    std::map<DWORD, KRewardItem> ItemInfoMap;   // +0x08  (key = daily index; observed 1 entry)
};                                  // sizeof = 0x20

// Per-grade reward info (size 0x1c).
struct KRewardInfo
{
    time_t  nLossDaily;             // +0x00  days-away threshold
    std::map<int, KRewardItemInfo> ItemMap;     // +0x04  key = kungfu id
};                                  // sizeof = 0x1c

class KRegressionManager
{
public:
    BOOL Init();
    void UnInit();

    BOOL   IsWork() { return m_bWorkFlag; }
    BOOL   IsCrossDays(time_t t1, time_t t2);
    time_t GetRegressionFinishedTime(time_t nNow);
    int    GetRewardGradeID(int nLossDays);
    KRewardInfo*     GetRewardInfo(int nGradeID);
    KRewardItemInfo* GetRewardItemInfo(int nGradeID, int nKungFu);

private:
    BOOL LoadConstList();
    BOOL LoadRegressionReward();
    BOOL CheckRegressionReward();

private:
    BOOL    m_bWorkFlag;        // +0x00
    time_t  m_nStartTime;       // +0x04
    time_t  m_nEndTime;         // +0x08
    time_t  m_nDailyCycle;      // +0x0c
    time_t  m_nDailyOffset;     // +0x10
    time_t  m_nResetCycle;      // +0x14
    time_t  m_nResetOffset;     // +0x18
    std::map<int, KRewardInfo> m_RewardMap;     // +0x1c  key = grade id
};

#endif  // _KREGRESSION_MANAGER_H_

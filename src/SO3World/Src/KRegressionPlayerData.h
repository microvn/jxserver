#ifndef _KREGRESSION_PLAYER_DATA_H_
#define _KREGRESSION_PLAYER_DATA_H_

//////////////////////////////////////////////////////////////////////////
// KRegressionPlayerData -- per-player returning-player (回归) state.
// Embedded in KPlayer; persisted as role-block rbtRegressionData.
// Layout + DB byte-format pinned from DWARF -- docs/regression_port/WORKLOG.md.
// PORTED: fields + persistence + read getters. DEFERRED (needs RE-4 confirm of
// the 2010 login post-load hook + VIP/kungfu accessors): Calculate (login
// re-grade), AddRewardItem + reward-claim script, nested-UI Lua, sync packet.
//////////////////////////////////////////////////////////////////////////

#define REGRESSION_ITEM_MARK_COUNT  8

class KPlayer;

class KRegressionPlayerData
{
public:
    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    // role-block: [account 22B][player 46B] = 68B (2010 combines the 2 v246 chunks)
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    int  GetGradeID()   { return m_nCurrentGradeID; }
    int  GetDailyCount(){ return m_nRegressionDailyCount; }
    BYTE GetItemMark(int nDailyIndex);

private:
    int    m_nAccountRegressionGradeID;     // +0x00
    int    m_nAccountRegressionVer;         // +0x04
    int    m_nPlayerAccountRegressionVer;   // +0x08  transient (not serialized)
    time_t m_nAccountRegressionEndTime;     // +0x0c
    time_t m_nNewPlayerRegressionEndTime;   // +0x10
    int    m_nCurrentGradeID;               // +0x14
    int    m_nRegressionDailyCount;         // +0x18
    BYTE   m_byItemMark[REGRESSION_ITEM_MARK_COUNT];  // +0x1c
    KPlayer* m_pPlayer;                     // +0x24
};

#endif  // _KREGRESSION_PLAYER_DATA_H_

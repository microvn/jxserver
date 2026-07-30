#ifndef _KREGRESSION_PLAYER_DATA_H_
#define _KREGRESSION_PLAYER_DATA_H_

//////////////////////////////////////////////////////////////////////////
// KRegressionPlayerData -- per-player returning-player (回归) state.
// Embedded in KPlayer; persisted as role-block rbtRegressionData.
// Layout + DB byte-format pinned from DWARF -- docs/regression_port/WORKLOG.md.
// PORTED: fields, persistence, account-data calculation and reward dispatch.
//////////////////////////////////////////////////////////////////////////

#define REGRESSION_ITEM_MARK_COUNT  8

class KPlayer;

class KRegressionPlayerData
{
public:
    KRegressionPlayerData();
    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    // role-block: [account 22B][player 46B] = 68B (2010 combines the 2 v246 chunks)
    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    // v246 role-data block contains only KREGRESSION_DB_DATA (46B).
    BOOL LoadPlayerData(BYTE* pbyData, size_t uDataLen);

    // v246 account-data chunk: [grade:1][end:4][version:1][reserved:16].
    BOOL LoadAccountData(BYTE* pbyData, size_t uDataLen);
    BOOL SaveAccountData(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);

    void Calculate(time_t nAccLastSaveTime, time_t nPlayerLastSaveTime);
    BOOL AddRewardItem(int nDailyIndex, int nItemIndex, DWORD dwKungFuID);
    BOOL CallAddRewardItemScript(DWORD dwItemType, DWORD dwItemIndex, int nItemStackNum);

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

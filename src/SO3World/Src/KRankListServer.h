#ifndef _KRANKLISTSERVER_H_
#define _KRANKLISTSERVER_H_

#include "Global.h"

// v2.5 NEW subsystem (RE'd from v246 SO3GameServerD). In-memory leaderboard cache for
// single-player-dungeon scores, indexed by force (门派) then rank. Fed by the center via an
// internal protocol (see docs/ranklist_port — feed handler DEFERRED, cross-process); queried
// by Lua (KPlayer method). No DB persistence (rebuilt from the sync feed).

#ifndef PLAYER_FORCE_MAX_COUNT
#define PLAYER_FORCE_MAX_COUNT  10   // v246 constant; 10 player forces (RelationForce id 1..10)
#endif
#define SCORE_RANK_MAX_COUNT    50   // v246 constant (0x32); top-50 per force

// Layout pinned from DWARF (KSingle_Dungeon_Score_Player_Info, byte_size 0x30 = 48).
struct KSingle_Dungeon_Score_Player_Info
{
    DWORD   dwPlayerID;         // @0x00
    char    szPlayerName[32];   // @0x04
    DWORD   dwMaxLevel;         // @0x24  (0 => empty slot; SyncFinish trims trailing zeros)
    DWORD   dwTotalScore;       // @0x28
    int     nEquipScore;        // @0x2c
};

class KRankListServer
{
public:
    KRankListServer();
    ~KRankListServer();

    BOOL Init();
    void UnInit();

    // Fed one entry at a time by the center's rank-push (dwForceID 1..10, dwRank 1..50).
    void SyncSingleDungeonScoreRankPlayerInfo(DWORD dwForceID, DWORD dwRank, KSingle_Dungeon_Score_Player_Info Info);
    // Batch-done: recompute each force's valid count (trailing empty slots removed).
    void SyncSingleDungeonScoreRankFinish();

    // Query a player's 1-based rank within their force; 0 = not ranked.
    int  GetSingleDungeonRankListPos(DWORD dwForceID, DWORD dwPlayerID);

private:
    // Layout mirrors DWARF (KRankListServer byte_size 0x5de8): count[] @0x00, info[][] @0x28.
    DWORD                             m_dwSingleDungeonScoreRankCount[PLAYER_FORCE_MAX_COUNT];
    KSingle_Dungeon_Score_Player_Info m_SingleDungeonScoreRankPlayerInfo[PLAYER_FORCE_MAX_COUNT][SCORE_RANK_MAX_COUNT];
};

#endif  //_KRANKLISTSERVER_H_

#include "stdafx.h"
#include "KRankListServer.h"

// v2.5 NEW (RE'd from v246). See KRankListServer.h + docs/ranklist_port/.

KRankListServer::KRankListServer()
{
}

KRankListServer::~KRankListServer()
{
}

// Init (v246 082130a6): zero the count[] block (0x28) and the info[][] block (24000).
BOOL KRankListServer::Init()
{
    memset(m_dwSingleDungeonScoreRankCount, 0, sizeof(m_dwSingleDungeonScoreRankCount));
    memset(m_SingleDungeonScoreRankPlayerInfo, 0, sizeof(m_SingleDungeonScoreRankPlayerInfo));
    return true;
}

void KRankListServer::UnInit()
{
    memset(m_dwSingleDungeonScoreRankCount, 0, sizeof(m_dwSingleDungeonScoreRankCount));
    memset(m_SingleDungeonScoreRankPlayerInfo, 0, sizeof(m_SingleDungeonScoreRankPlayerInfo));
}

// SyncPlayerInfo (v246 08212f6e): store one entry at [force-1][rank-1].
void KRankListServer::SyncSingleDungeonScoreRankPlayerInfo(DWORD dwForceID, DWORD dwRank, KSingle_Dungeon_Score_Player_Info Info)
{
    KGLOG_PROCESS_ERROR(dwForceID > 0 && dwForceID <= PLAYER_FORCE_MAX_COUNT);
    KGLOG_PROCESS_ERROR(dwRank > 0 && dwRank <= SCORE_RANK_MAX_COUNT);

    m_SingleDungeonScoreRankPlayerInfo[dwForceID - 1][dwRank - 1] = Info;

Exit0:
    return;
}

// SyncFinish (v246 08212e24): per force, count = 50 minus the trailing run of empty slots
// (dwMaxLevel == 0). Ranks are pre-sorted top-down by the feeder, so empties cluster at the tail.
void KRankListServer::SyncSingleDungeonScoreRankFinish()
{
    int nForce = 0;
    int nRank  = 0;

    for (nForce = PLAYER_FORCE_MAX_COUNT - 1; nForce >= 0; nForce--)
    {
        m_dwSingleDungeonScoreRankCount[nForce] = SCORE_RANK_MAX_COUNT;

        for (nRank = SCORE_RANK_MAX_COUNT - 1;
             nRank >= 0 && m_SingleDungeonScoreRankPlayerInfo[nForce][nRank].dwMaxLevel == 0;
             nRank--)
        {
            m_dwSingleDungeonScoreRankCount[nForce]--;
        }
    }
}

// GetRankListPos (v246 08212e9e): 1-based rank of dwPlayerID within its force; 0 = not ranked.
int KRankListServer::GetSingleDungeonRankListPos(DWORD dwForceID, DWORD dwPlayerID)
{
    int nResult = 0;
    int nRank   = 0;

    KGLOG_PROCESS_ERROR(dwPlayerID > 0);
    KGLOG_PROCESS_ERROR(dwForceID > 0 && dwForceID <= PLAYER_FORCE_MAX_COUNT);

    for (nRank = 0; nRank < SCORE_RANK_MAX_COUNT; nRank++)
    {
        if (m_SingleDungeonScoreRankPlayerInfo[dwForceID - 1][nRank].dwPlayerID == dwPlayerID)
        {
            nResult = nRank + 1;
            break;
        }
    }

Exit0:
    return nResult;
}

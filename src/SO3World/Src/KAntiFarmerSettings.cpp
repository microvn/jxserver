#include "stdafx.h"
#include "Global.h"
#include "KAntiFarmerSettings.h"

KAntiFarmerSettings::KAntiFarmerSettings()
{
    // Target constructs the set first and initializes only m_bEnable here.
    m_bEnable = false;
}

BOOL KAntiFarmerSettings::AppendIgnoreNpcTemplateID(DWORD dwTemplateID)
{
    return m_NpcWhiteList.insert(static_cast<unsigned long>(dwTemplateID)).second;
}

BOOL KAntiFarmerSettings::UnInit()
{
    m_NpcWhiteList.clear();
    return true;
}

BOOL KAntiFarmerSettings::LoadNpcTemplateIDWiteList()
{
    BOOL bResult = false;
    ITabFile* pTabFile = NULL;
    char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "AntiFarmerWhiteList.tab");
    szFile[sizeof(szFile) - 1] = '\0';
    pTabFile = g_OpenTabFile(szFile);
    KGLOG_PROCESS_ERROR(pTabFile);
    for (int nLine = 2; nLine < pTabFile->GetHeight(); ++nLine)
    {
        DWORD dwTemplateID = 0;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, "NpcTemplateID", 0,
            (int*)&dwTemplateID));
        // Target inserts into the set and ignores the duplicate result.
        m_NpcWhiteList.insert(static_cast<unsigned long>(dwTemplateID));
    }
    bResult = true;
Exit0:
    if (pTabFile) pTabFile->Release();
    return bResult;
}

BOOL KAntiFarmerSettings::Init()
{
    BOOL bResult = false;
    IIniFile* pIniFile = NULL;
    char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "AntiFarmer.ini");
    szFile[sizeof(szFile) - 1] = '\0';
    pIniFile = g_OpenIniFile(szFile);
    KGLOG_PROCESS_ERROR(pIniFile);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "Enable", 0,
        (int*)&m_bEnable));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "EffectiveTime", 864000,
        (int*)&m_nEffectiveTime));
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "CheckOnlineTimeRange", 21600,
        &m_nCheckOnlineTimeRange));
    KGLOG_PROCESS_ERROR(m_nEffectiveTime > 3599);
    KGLOG_PROCESS_ERROR(pIniFile->GetFloat("Global", "OnlineRate", .9f,
        &m_fOnlineRate));
    KGLOG_PROCESS_ERROR(m_nCheckOnlineTimeRange > 30);
    KGLOG_PROCESS_ERROR(pIniFile->GetFloat("Global", "KillNpcRate", .95f,
        &m_fKillNpcRate));
    KGLOG_PROCESS_ERROR(m_fOnlineRate > 0.0f && m_fOnlineRate <= 1.0f);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "KillNpcTypeNum", 3,
        &m_nKillNpcTypeNum));
    KGLOG_PROCESS_ERROR(m_fKillNpcRate > 0.0f && m_fKillNpcRate <= 1.0f);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "RecordKillNpcNum", 600,
        &m_nRecordKillNpcNum));
    KGLOG_PROCESS_ERROR(m_nKillNpcTypeNum > 0 && m_nKillNpcTypeNum <= 10);
    KGLOG_PROCESS_ERROR(pIniFile->GetFloat("Global", "PunishRate", .8f,
        &m_fPunishRate));
    KGLOG_PROCESS_ERROR(m_nRecordKillNpcNum > 0 &&
        m_nKillNpcTypeNum <= m_nRecordKillNpcNum);
    KGLOG_PROCESS_ERROR(m_fPunishRate >= 0.0f && m_fPunishRate <= 1.0f);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "DetectSameBehaviorDistanceRange", 384,
        &m_nDetectSameBehaviorDistanceRange));
    KGLOG_PROCESS_ERROR(m_nDetectSameBehaviorDistanceRange >= 0);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "DetectAroundSameBehaviorThreshold", 5,
        &m_nDetectAroundSameBehaviorThreshold));
    KGLOG_PROCESS_ERROR(m_nDetectAroundSameBehaviorThreshold > 0);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "DetectProcessSpeed", 5,
        &m_nDetectProcessSpeed));
    KGLOG_PROCESS_ERROR(m_nDetectProcessSpeed > 0);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "PunishFreezeFarmerNum", 10,
        &m_nPunishFreezeFarmerNum));
    KGLOG_PROCESS_ERROR(m_nPunishFreezeFarmerNum > 0);
    KGLOG_PROCESS_ERROR(pIniFile->GetFloat("Global", "EquipSimilarityThreshold", .9f,
        &m_fEquipSimilarityThreshold));
    KGLOG_PROCESS_ERROR(m_fEquipSimilarityThreshold >= 0.5f &&
        m_fEquipSimilarityThreshold <= 1.0f);
    KGLOG_PROCESS_ERROR(pIniFile->GetInteger("Global", "EnableIPRule", 0,
        (int*)&m_bEnableIPRule));
    KGLOG_PROCESS_ERROR(LoadNpcTemplateIDWiteList());
    bResult = true;
Exit0:
    if (pIniFile) pIniFile->Release();
    return bResult;
}

BOOL KAntiFarmerSettings::NeedIgnore(DWORD dwTemplateID) const
{
    return m_NpcWhiteList.find(static_cast<unsigned long>(dwTemplateID)) != m_NpcWhiteList.end();
}

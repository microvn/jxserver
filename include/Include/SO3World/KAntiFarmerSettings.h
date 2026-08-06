#ifndef _KANTI_FARMER_SETTINGS_H_
#define _KANTI_FARMER_SETTINGS_H_

#include <set>
#include <ctime>

class KAntiFarmerSettings
{
public:
    KAntiFarmerSettings();
    BOOL Init();
    BOOL UnInit();
    BOOL LoadNpcTemplateIDWiteList();
    BOOL AppendIgnoreNpcTemplateID(DWORD dwTemplateID);
    BOOL NeedIgnore(DWORD dwTemplateID) const;

    BOOL m_bEnable;
    time_t m_nEffectiveTime;
    int m_nCheckOnlineTimeRange;
    float m_fOnlineRate;
    float m_fKillNpcRate;
    int m_nKillNpcTypeNum;
    int m_nRecordKillNpcNum;
    float m_fPunishRate;
    int m_nDetectSameBehaviorDistanceRange;
    int m_nDetectAroundSameBehaviorThreshold;
    int m_nDetectProcessSpeed;
    int m_nPunishFreezeFarmerNum;
    float m_fEquipSimilarityThreshold;
    BOOL m_bEnableIPRule;

private:
    std::set<unsigned long> m_NpcWhiteList;
};

#endif

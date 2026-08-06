#ifndef _KNPC_ADRON_TAB_H_
#define _KNPC_ADRON_TAB_H_
#include <map>
#include "Engine/KMemory.h"
#include "KAttribute.h"
class KNpc; class KScene;
struct KADRON_KEY { DWORD dwMapKey; DWORD dwTemplateID; bool operator<(const KADRON_KEY& rhs) const; };
struct KADRON_INFO { int nLevel; int nContribution; int nAchievementID; KAttribute* pAttributeList; };
typedef std::map<KADRON_KEY, KADRON_INFO, std::less<KADRON_KEY>, KMemory::KAllocator<std::pair<const KADRON_KEY, KADRON_INFO> > > KADRON_MAP;
class KNpcAdronTab
{
public:
    KNpcAdronTab(); ~KNpcAdronTab(); BOOL Init(); BOOL UnInit();
    BOOL AdronNpcIntensity(KNpc* pNpc, KScene* pScene); const KADRON_INFO* GetAdronInfo(DWORD, DWORD);
private: KADRON_MAP m_NpcAdronMap;
};
#endif

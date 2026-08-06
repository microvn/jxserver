#include "stdafx.h"
#include "Global.h"
#include "KNpcAdronTab.h"
#include "KNpc.h"
#include "KScene.h"
namespace
{
BOOL LoadAdronTabAttribute(ITabFile* pTabFile, int nLine, int nIndex, int& rnID, int& rnA, int& rnB)
{
    char szA[64], szVA[64], szVB[64], szValue[64];
    assert(pTabFile);
    snprintf(szA, sizeof(szA), "Attrib%d", nIndex); snprintf(szVA, sizeof(szVA), "Value%dA", nIndex); snprintf(szVB, sizeof(szVB), "Value%dB", nIndex);
    KGLOG_PROCESS_ERROR(pTabFile->GetString(nLine, szA, "", szValue, sizeof(szValue)) >= 0);
    KGLOG_PROCESS_ERROR(pTabFile->GetString(nLine, szVA, "?", szValue, sizeof(szValue)) > 0);
    KGLOG_PROCESS_ERROR(AttributeStringToID(szValue, rnID)); KGLOG_PROCESS_ERROR(rnID != 0x149);
    KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, szVA, 0, &rnA)); KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine, szVB, 0, &rnB)); return true;
Exit0: return false;
}
}
bool KADRON_KEY::operator<(const KADRON_KEY& rhs) const { return dwMapKey != rhs.dwMapKey ? dwMapKey < rhs.dwMapKey : dwTemplateID < rhs.dwTemplateID; }
KNpcAdronTab::KNpcAdronTab()
{
    // The target constructs the map and explicitly clears it in the
    // constructor before returning.
    m_NpcAdronMap.clear();
}
KNpcAdronTab::~KNpcAdronTab() { assert(m_NpcAdronMap.empty()); }
BOOL KNpcAdronTab::UnInit()
{
    for (KADRON_MAP::iterator it = m_NpcAdronMap.begin(); it != m_NpcAdronMap.end(); ++it)
        for (KAttribute* p = it->second.pAttributeList; p; ) { KAttribute* pNext = p->pNext; KMemory::Delete(p); p = pNext; }
    m_NpcAdronMap.clear(); return true;
}
const KADRON_INFO* KNpcAdronTab::GetAdronInfo(DWORD dwMapKey, DWORD dwTemplateID)
{
    KADRON_KEY key = {dwMapKey, dwTemplateID}; KADRON_MAP::iterator it = m_NpcAdronMap.find(key); return it == m_NpcAdronMap.end() ? NULL : &it->second;
}
BOOL KNpcAdronTab::AdronNpcIntensity(KNpc* pNpc, KScene* pScene)
{
    // Target AdronNpcIntensity @0x081de126 lets both guards jump to the common exit
    // (0x081de266) past the KADRON_INFO locals, so the lookup result must be declared
    // before the guards: C++98 (GCC 4.8.5 -std=gnu++98) forbids a goto crossing the
    // initialisation of a local.
    // PORT-TODO[TARGET_REQUIRED]: the target keys on {pScene->m_dwMapID (KScene+0x8),
    // pNpc->m_dwTemplateID (KNpc+0xa54)} rather than KScene::MakeMapKey, and writes
    // pNpc->m_nLevel (KCharacter+0x90, store at 0x081de240) rather than
    // pScene->m_nAOECountPercent (KScene+0x90). Owned by the KNpcAdronTab port, stopped
    // edge SE-3; not changed by the build gate.
    const KADRON_INFO* pInfo = NULL;
    KGLOG_PROCESS_ERROR(pNpc); KGLOG_PROCESS_ERROR(pScene);
    pInfo = GetAdronInfo(KScene::MakeMapKey(pScene->m_dwMapID, pScene->m_nCopyIndex), pNpc->m_dwTemplateID);
    if (pInfo)
    {
        if (pInfo->nLevel)
            pScene->m_nAOECountPercent = pInfo->nLevel;
        if (pInfo->pAttributeList)
            pNpc->ApplyAttribute(pInfo->pAttributeList);
    }
    return true;
Exit0: return false;
}
BOOL KNpcAdronTab::Init()
{
    BOOL bResult = false; ITabFile* pTabFile = NULL; char szFile[MAX_PATH];
    snprintf(szFile, sizeof(szFile), "%s/%s", SETTING_DIR, "NpcAdron.tab"); szFile[sizeof(szFile)-1] = '\0'; pTabFile = g_OpenTabFile(szFile); KGLOG_PROCESS_ERROR(pTabFile);
    KGLOG_PROCESS_ERROR(pTabFile->GetHeight() > 1);
    for (int nLine = 2; nLine <= pTabFile->GetHeight(); ++nLine)
    {
        KADRON_INFO info = {0,0,0,NULL}; int nMapID=0,nCopy=0,nTemplate=0;
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"MapID",0,&nMapID)); KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"CopyIndex",0,&nCopy)); KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"NpcTemplateID",0,&nTemplate));
        KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"Level",0,&info.nLevel)); KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"Contribution",0,&info.nContribution)); KGLOG_PROCESS_ERROR(pTabFile->GetInteger(nLine,"AchievementID",0,&info.nAchievementID));
        KAttribute* pTail=NULL;
        for (int nIndex=1;;++nIndex) { int nID=0,nA=0,nB=0; if (!LoadAdronTabAttribute(pTabFile,nLine,nIndex,nID,nA,nB)) break; KAttribute* p=KMemory::New<KAttribute>(); KGLOG_PROCESS_ERROR(p); p->nKey=nID;p->nValue1=nA;p->nValue2=nB;p->pNext=NULL;if(!info.pAttributeList)info.pAttributeList=p;else pTail->pNext=p;pTail=p; }
        KADRON_KEY key={KScene::MakeMapKey((DWORD)nMapID,nCopy),(DWORD)nTemplate}; KGLOG_PROCESS_ERROR(m_NpcAdronMap.insert(std::make_pair(key,info)).second); info.pAttributeList=NULL;
    }
    bResult=true;
Exit0: if(pTabFile)pTabFile->Release(); if(!bResult)UnInit(); return bResult;
}

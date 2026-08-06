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
    /* Target LoadAdronTabAttribute @081de453.  Two defects fixed here:
         1. the probe is FindColumn(szAttrib) (ITabFile vtable slot 0x10, call at
            081de526), not GetString; `js` on the result returns false SILENTLY.
            This call is the loop terminator for every row, so routing it through
            KGLOG_PROCESS_ERROR logged an error on each row of NpcAdron.tab.
         2. the attribute NAME is read from the "Attrib%d" column: the callsite at
            081de575 passes the buffer at -0x48 (szAttrib) as the column and
            -0x108 (szValue) as the output.  The candidate passed szVA
            ("Value%dA") as the column, so AttributeStringToID was fed the
            numeric value ("70") instead of "atMaxLifeBase" - it always failed
            and no adron attribute was ever loaded.
       Silent (KG_PROCESS_ERROR) vs logged: only the AttributeStringToID failure
       at 081de5c9 uses the KGLOG_PROCESS_ERROR format string; the FindColumn,
       GetString and two GetInteger failures have no KGLogPrintf at all. */
    KG_PROCESS_ERROR(pTabFile->FindColumn(szA) >= 0);
    KG_PROCESS_ERROR(pTabFile->GetString(nLine, szA, "", szValue, sizeof(szValue)) > 0);
    KGLOG_PROCESS_ERROR(AttributeStringToID(szValue, rnID));
    if (rnID == atExecuteScript)   // KAttribute.h:347 == 329 == 0x149
    {
        // Target 081de5d8 cmpl $0x149 -> KGLogPrintf(KGLOG_DEBUG, ...) at
        // 081de5fc with (nIndex, nLine), then return false.
        KGLogPrintf(KGLOG_DEBUG,
            "Load NpcAdron.tab Error By AttribIndex %d Type is atExecuteScript at line %d.\n",
            nIndex, nLine);
        goto Exit0;
    }
    KG_PROCESS_ERROR(pTabFile->GetInteger(nLine, szVA, 0, &rnA));
    KG_PROCESS_ERROR(pTabFile->GetInteger(nLine, szVB, 0, &rnB));
    return true;
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
    // PORT-DEFERRED_WIRING[CALLER]: target semantics are FULLY recovered (below); what
    // is deferred is the authorisation to change the write target, not the evidence.
    // The target keys on {pScene->m_dwMapID (KScene+0x8),
    // pNpc->m_dwTemplateID (KNpc+0xa54)} rather than KScene::MakeMapKey, and writes
    // pNpc->m_nLevel (KCharacter+0x90, store at 0x081de240) rather than
    // pScene->m_nAOECountPercent (KScene+0x90). Owned by the KNpcAdronTab port, stopped
    // edge SE-3; not changed by the build gate.
    // Re-confirmed 2026-08-07 from the target operands: the lookup key is built from
    // KScene+0x8 read directly at 081de1af with NO call to KScene::MakeMapKey, and the
    // level store at 081de240 targets the KNpc at +0x90. That asymmetry against Init
    // (which DOES key inserts on MakeMapKey(MapID, MapLevel) via the call at 081de95c)
    // is a target fact and must not be "fixed".
    // REACHABILITY, measured 2026-08-07: AdronNpcIntensity has ZERO callers in the whole
    // candidate tree (`rg -n AdronNpcIntensity src include` returns only this definition
    // and its declaration), and zero callers in the target call graph. KWorldSettings
    // calls only m_NpcAdronTab.Init()/UnInit(). Populating the map does not make this
    // body execute. A blind review read the fixed population as making the path "live";
    // that is not supported - liveness needs a caller, and there is none in either tree.
    // The write below is nonetheless WRONG against the target and must not be treated as
    // ported: it is dead-but-wrong, not dead-and-harmless.
    // owner=KNpcAdronTab port / ticket w1-npcadron-intensity
    // unblock_condition=KNpc exposes the KCharacter+0x90 level field and KNpc+0xa54 is
    //   confirmed to be m_dwTemplateID by DWARF, AND a ticket authorises changing the
    //   AdronNpcIntensity write target (it is outside this ticket's allowlist intent).
    //   Re-open immediately if any caller of AdronNpcIntensity is ever added.
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
    // Target 081de785: SetErrorLog(0) (ITabFile vtable slot 0x08, argument 0)
    // immediately after the open and before the first GetHeight.
    pTabFile->SetErrorLog(0);
    KGLOG_PROCESS_ERROR(pTabFile->GetHeight() > 1);   // 081de79c cmpl $0x1 / jg
    for (int nLine = 2; nLine <= pTabFile->GetHeight(); ++nLine)
    {
        KADRON_INFO info = {0,0,0,NULL}; int nMapID=0,nCopy=0,nTemplate=0;
        /* Column names taken from the callsite operands: "MapID" (081de823),
           "MapLevel" (081de85b) and "TemplateID" (081de896) - the candidate used
           "CopyIndex" and "NpcTemplateID", which do not exist in NpcAdron.tab
           (header: MapID MapLevel TemplateID Level Contribution AchievementID).
           None of the six reads is checked in the target: no cmp or branch
           follows any of the indirect calls, so a blank cell keeps the 0 default
           instead of failing the whole settings load. */
        pTabFile->GetInteger(nLine,"MapID",0,&nMapID);
        pTabFile->GetInteger(nLine,"MapLevel",0,&nCopy);
        pTabFile->GetInteger(nLine,"TemplateID",0,&nTemplate);
        pTabFile->GetInteger(nLine,"Level",0,&info.nLevel);
        pTabFile->GetInteger(nLine,"Contribution",0,&info.nContribution);
        pTabFile->GetInteger(nLine,"AchievementID",0,&info.nAchievementID);
        KAttribute* pTail=NULL;
        for (int nIndex=1;;++nIndex) { int nID=0,nA=0,nB=0; if (!LoadAdronTabAttribute(pTabFile,nLine,nIndex,nID,nA,nB)) break; KAttribute* p=KMemory::New<KAttribute>(); KGLOG_PROCESS_ERROR(p); p->nKey=nID;p->nValue1=nA;p->nValue2=nB;p->pNext=NULL;if(!info.pAttributeList)info.pAttributeList=p;else pTail->pNext=p;pTail=p; }
        // Target 081deac8: the map::insert result is discarded (no test of the
        // returned pair), so a duplicate {MapKey,TemplateID} keeps the first row.
        KADRON_KEY key={KScene::MakeMapKey((DWORD)nMapID,nCopy),(DWORD)nTemplate};
        m_NpcAdronMap.insert(std::make_pair(key,info));
        info.pAttributeList=NULL;
    }
    bResult=true;
Exit0: if(pTabFile)pTabFile->Release(); if(!bResult)UnInit(); return bResult;
}

#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KHairShop.h"

//////////////////////////////////////////////////////////////////////////
// KHairShop config loader. Column -> field mapping and the load-time
// asserts (Price==0, Coin>=0) are pinned from v246 LoadHairIndexTable
// (081ee3ca) / LoadHairPriceTable (081edede) -- see WORKLOG [RE-2].
//////////////////////////////////////////////////////////////////////////

#define HAIRSHOP_SUB_DIR        "HairShop"
#define HAIR_INDEX_TAB          "HeadIndex.tab"
#define HAIR_PRICE_TAB          "HairPrice.tab"

#define HAIR_DUP_LOG(file, line, id) \
    KGLogPrintf(KGLOG_DEBUG, "[SO3World][HairShop] In table \"%s\", line %d: ID %u already exist! Please check your table file.", (file), (line), (id))

BOOL KHairShop::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    m_bCloseHairShop = false;

    bRetCode = LoadHairIndexTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadHairPriceTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KHairShop::UnInit()
{
    m_HairIndexInfoMap.clear();
    m_HairPriceInfoMap.clear();
}

//------------------------------------------------------------------------
// HeadIndex.tab -> m_HairIndexInfoMap<HeadID, KHAIR_INDEX_INFO>
// Column order (Headform, Bang, Plait) == struct memory order (RE-2).
//------------------------------------------------------------------------
BOOL KHairShop::LoadHairIndexTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, HAIRSHOP_SUB_DIR, HAIR_INDEX_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[HairShop] Unable to open HeadIndex.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int              nHeadID = 0;
        KHAIR_INDEX_INFO Info;

        memset(&Info, 0, sizeof(Info));

        nRetCode = piTabFile->GetInteger(nLine, "HeadID",     0, &nHeadID);                    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "HeadformID", 0, (int*)&Info.dwHeadformID);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "BangID",     0, (int*)&Info.dwBangID);        KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "PlaitID",    0, (int*)&Info.dwPlaitID);       KGLOG_PROCESS_ERROR(nRetCode);

        if (m_HairIndexInfoMap.find((DWORD)nHeadID) != m_HairIndexInfoMap.end())
        {
            HAIR_DUP_LOG(szFileName, nLine, nHeadID);
            goto Exit0;
        }
        m_HairIndexInfoMap[(DWORD)nHeadID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", HAIR_INDEX_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// HairPrice.tab -> m_HairPriceInfoMap<{RoleType,Type,ID}, KHAIR_PRICE_INFO>
// v246 asserts Price == 0 (hair is not bought with in-game money) and
// Coin >= 0 -- kept verbatim (values-from-binary).
//------------------------------------------------------------------------
BOOL KHairShop::LoadHairPriceTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, HAIRSHOP_SUB_DIR, HAIR_PRICE_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[HairShop] Unable to open HairPrice.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        KHAIR_PRICE_KEY  Key;
        KHAIR_PRICE_INFO Info;

        memset(&Key,  0, sizeof(Key));
        memset(&Info, 0, sizeof(Info));

        nRetCode = piTabFile->GetInteger(nLine, "RoleType",      0, &Key.nRoleType);        KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Type",          0, &Key.nType);            KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ID",            0, (int*)&Key.dwID);       KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Price",         0, &Info.nPrice);          KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Coin",          0, &Info.nCoin);           KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "NeedFreeCount", 0, &Info.nNeedFreeCount);  KGLOG_PROCESS_ERROR(nRetCode);

        KGLOG_PROCESS_ERROR(Info.nPrice == 0);
        KGLOG_PROCESS_ERROR(Info.nCoin >= 0);

        if (m_HairPriceInfoMap.find(Key) != m_HairPriceInfoMap.end())
        {
            HAIR_DUP_LOG(szFileName, nLine, Key.dwID);
            goto Exit0;
        }
        m_HairPriceInfoMap[Key] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", HAIR_PRICE_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// Query accessors
//------------------------------------------------------------------------
KHAIR_INDEX_INFO* KHairShop::GetHairIndexInfo(DWORD dwHeadID)
{
    KHAIR_INDEX_INFO*         pResult = NULL;
    KHAIR_INDEX_MAP::iterator it      = m_HairIndexInfoMap.find(dwHeadID);

    KGLOG_PROCESS_ERROR(it != m_HairIndexInfoMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

KHAIR_PRICE_INFO* KHairShop::GetPriceInfo(int nRoleType, int nType, DWORD dwID)
{
    KHAIR_PRICE_INFO*         pResult = NULL;
    KHAIR_PRICE_KEY           Key;
    KHAIR_PRICE_MAP::iterator it;

    Key.nRoleType = nRoleType;
    Key.nType     = nType;
    Key.dwID      = dwID;

    it = m_HairPriceInfoMap.find(Key);
    KGLOG_PROCESS_ERROR(it != m_HairPriceInfoMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

//------------------------------------------------------------------------
// Grant path (v246 AddHair 081ed3fa / ChangePlayerHair 081ed5cc, money-free).
// AddHair = give to box (validates shop) + apply appearance. The buy chain
// (Buy/Pay/OnHairShopBuyHairRespond) is deferred -- needs the coin subsystem.
//------------------------------------------------------------------------
BOOL KHairShop::AddHair(KPlayer* pPlayer, int nType, DWORD dwID)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    if (dwID == 0)
        return true;

    KGLOG_PROCESS_ERROR(pPlayer->m_HairBox.Add(nType, dwID));    // own it (validates shop table)
    pPlayer->m_HairBox.ChangeHair(nType, dwID);                  // apply onto appearance

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHairShop::ChangePlayerHair(KPlayer* pPlayer, DWORD dwHairID, DWORD dwFaceID)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);

    if (dwHairID != 0)
        KGLOG_PROCESS_ERROR(AddHair(pPlayer, hsHair, dwHairID));
    if (dwFaceID != 0)
        KGLOG_PROCESS_ERROR(AddHair(pPlayer, hsFace, dwFaceID));

    bResult = true;
Exit0:
    return bResult;
}

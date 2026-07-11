#include "stdafx.h"
#include "Global.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KExterior.h"

//////////////////////////////////////////////////////////////////////////
// KExterior config loader. Column -> field mapping and the *10000 / range
// validations are pinned from v246 (see WORKLOG [RE-6]/[PORT-1]/[PORT-2]).
//////////////////////////////////////////////////////////////////////////

#define EXTERIOR_SUB_DIR            "Exterior"
#define EXTERIOR_INFO_TAB           "ExteriorInfo.tab"
#define EXTERIOR_SUIT_INFO_TAB      "ExteriorSuitInfo.tab"
#define EXTERIOR_INDEX_TAB          "ExteriorBuy.tab"       // reverse-lookup source
#define EXTERIOR_SHOP_PRICE_TAB     "Exteriorshop.tab"

// Value pinned from the suit-loader compare immediate in FUN_081fad0a.
#define MAX_EXTERIOR_ACHIEVEMENT_ID 4000

#define EXTERIOR_DUP_LOG(file, line, id) \
    KGLogPrintf(KGLOG_DEBUG, "[SO3World][Exterior] In table \"%s\", line %d: ID %u already exist! Please check your table file.", (file), (line), (id))

BOOL KExterior::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadExteriorInfoTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadExteriorSuitInfoTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadExteriorIndexInfoTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadExteriorShopPriceTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KExterior::UnInit()
{
    m_ExteriorInfoMap.clear();
    m_ExteriorSuitMap.clear();
    m_ExteriorIndexMap.clear();
    m_ExteriorShopMap.clear();
}

//------------------------------------------------------------------------
// ExteriorInfo.tab -> m_ExteriorInfoMap<ID, KEXTERIOR_INFO>
//------------------------------------------------------------------------
BOOL KExterior::LoadExteriorInfoTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, EXTERIOR_SUB_DIR, EXTERIOR_INFO_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[Exterior] Unable to open ExteriorInfo.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int             nID = 0;
        KEXTERIOR_INFO  Info;
        int             i = 0;

        memset(&Info, 0, sizeof(Info));

        nRetCode = piTabFile->GetInteger(nLine, "ID",            0, &nID);                     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ForceID",       0, (int*)&Info.nForceID);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Genre",         0, (int*)&Info.nGenre);       KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "set",           0, (int*)&Info.nSet);         KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "subtype",       0, (int*)&Info.nSubType);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "represetID",    0, (int*)&Info.nRepresentID); KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "colorID",       0, (int*)&Info.nColorID);     KGLOG_PROCESS_ERROR(nRetCode);
        // nPrice[time][pay]: time Permanent=0/7days=1/Limit=2 ; pay Free=0/Money=1/Coin=2
        nRetCode = piTabFile->GetInteger(nLine, "7days0",        0, &Info.nPrice[ett7Days][epctMoney]);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "7days1",        0, &Info.nPrice[ett7Days][epctCoin]);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "7daysFree",     0, &Info.nPrice[ett7Days][epctFree]);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Permanent0",    0, &Info.nPrice[ettPermanent][epctMoney]); KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Permanent1",    0, &Info.nPrice[ettPermanent][epctCoin]);  KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "PermanentFree", 0, &Info.nPrice[ettPermanent][epctFree]);  KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limittype",     0, (int*)&Info.nLimitType);  KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limitprice0",   0, &Info.nPrice[ettLimit][epctMoney]);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limitprice1",   0, &Info.nPrice[ettLimit][epctCoin]);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "LimitpriceFree",0, &Info.nPrice[ettLimit][epctFree]);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "IconID",        0, (int*)&Info.nIconID);      KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "RepresetID1",   0, (int*)&Info.nRepresentID1); KGLOG_PROCESS_ERROR(nRetCode);

        for (i = 0; i < ettTotal; i++)
        {
            if (Info.nPrice[i][epctMoney] > 0)
            {
                Info.nPrice[i][epctMoney] *= 10000;
                KGLOG_PROCESS_ERROR(Info.nPrice[i][epctMoney] > 0);
            }
        }

        if (m_ExteriorInfoMap.find((DWORD)nID) != m_ExteriorInfoMap.end())
        {
            EXTERIOR_DUP_LOG(szFileName, nLine, nID);
            goto Exit0;
        }
        m_ExteriorInfoMap[(DWORD)nID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", EXTERIOR_INFO_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// ExteriorSuitInfo.tab -> m_ExteriorSuitMap<SuitID, KEXTERIOR_SUIT_INFO>
//------------------------------------------------------------------------
BOOL KExterior::LoadExteriorSuitInfoTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, EXTERIOR_SUB_DIR, EXTERIOR_SUIT_INFO_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[Exterior] Unable to open ExteriorSuitInfo.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int                 nSuitID        = 0;
        int                 nNeedPermanent = 0;
        KEXTERIOR_SUIT_INFO Info;

        memset(&Info, 0, sizeof(Info));

        nRetCode = piTabFile->GetInteger(nLine, "SuitID",        0, &nSuitID);              KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Chest",         0, (int*)&Info.nChest);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Helm",          0, (int*)&Info.nHelm);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Waist",         0, (int*)&Info.nWaist);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Boots",         0, (int*)&Info.nBoots);    KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Bangle",        0, (int*)&Info.nBangle);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "AchievementID", -1, &Info.nAchievementID); KGLOG_PROCESS_ERROR(nRetCode);
        KGLOG_PROCESS_ERROR(Info.nAchievementID >= 0 && Info.nAchievementID < MAX_EXTERIOR_ACHIEVEMENT_ID);
        nRetCode = piTabFile->GetInteger(nLine, "NeedPermanent", 0, &nNeedPermanent);       KGLOG_PROCESS_ERROR(nRetCode);
        Info.bNeedPermanent = (nNeedPermanent > 0);

        if (m_ExteriorSuitMap.find((DWORD)nSuitID) != m_ExteriorSuitMap.end())
        {
            EXTERIOR_DUP_LOG(szFileName, nLine, nSuitID);
            goto Exit0;
        }
        m_ExteriorSuitMap[(DWORD)nSuitID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", EXTERIOR_SUIT_INFO_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// ExteriorBuy.tab -> m_ExteriorIndexMap<{subtype,represent,color,force}, ID>
// (reverse lookup: appearance -> exterior ID)
//------------------------------------------------------------------------
BOOL KExterior::LoadExteriorIndexInfoTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, EXTERIOR_SUB_DIR, EXTERIOR_INDEX_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[Exterior] Unable to open ExteriorBuy.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int                 nID = 0;
        KEXTERIOR_INDEX_KEY Key;

        memset(&Key, 0, sizeof(Key));

        nRetCode = piTabFile->GetInteger(nLine, "SubType",     0, (int*)&Key.nSubType);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "RepresentID", 0, (int*)&Key.nRepresentID); KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ColorID",     0, (int*)&Key.nColorID);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ForceID",     0, (int*)&Key.nForceID);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "ID",          0, &nID);                    KGLOG_PROCESS_ERROR(nRetCode);

        if (m_ExteriorIndexMap.find(Key) != m_ExteriorIndexMap.end())
        {
            EXTERIOR_DUP_LOG(szFileName, nLine, nID);
            goto Exit0;
        }
        m_ExteriorIndexMap[Key] = (DWORD)nID;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", EXTERIOR_INDEX_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// Exteriorshop.tab -> m_ExteriorShopMap<ID, KEXTERIOR_SHOP_PRICE>
//------------------------------------------------------------------------
BOOL KExterior::LoadExteriorShopPriceTable()
{
    BOOL        bResult      = false;
    int         nRetCode     = false;
    ITabFile*   piTabFile    = NULL;
    int         nLine        = 0;
    int         nHeight      = 0;
    char        szFileName[MAX_PATH];

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s/%s", SETTING_DIR, EXTERIOR_SUB_DIR, EXTERIOR_SHOP_PRICE_TAB);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFileName));

    piTabFile = g_OpenTabFile(szFileName);
    KGLOG_PROCESS_ERROR(piTabFile && "[Exterior] Unable to open Exteriorshop.tab");

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (nLine = 2; nLine <= nHeight; nLine++)
    {
        int                  nID = 0;
        KEXTERIOR_SHOP_PRICE Price;
        int                  i = 0;

        memset(&Price, 0, sizeof(Price));

        nRetCode = piTabFile->GetInteger(nLine, "ID",            0, &nID);                        KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "7days0",        0, &Price.nPrice[ett7Days][epctMoney]);      KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "7days1",        0, &Price.nPrice[ett7Days][epctCoin]);       KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "7daysFree",     0, &Price.nPrice[ett7Days][epctFree]);       KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Permanent0",    0, &Price.nPrice[ettPermanent][epctMoney]);  KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Permanent1",    0, &Price.nPrice[ettPermanent][epctCoin]);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "PermanentFree", 0, &Price.nPrice[ettPermanent][epctFree]);   KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limittype",     0, (int*)&Price.nLimitType);     KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limitprice0",   0, &Price.nPrice[ettLimit][epctMoney]);      KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "Limitprice1",   0, &Price.nPrice[ettLimit][epctCoin]);       KGLOG_PROCESS_ERROR(nRetCode);
        nRetCode = piTabFile->GetInteger(nLine, "LimitpriceFree",0, &Price.nPrice[ettLimit][epctFree]);       KGLOG_PROCESS_ERROR(nRetCode);

        for (i = 0; i < ettTotal; i++)
        {
            if (Price.nPrice[i][epctMoney] > 0)
            {
                Price.nPrice[i][epctMoney] *= 10000;
                KGLOG_PROCESS_ERROR(Price.nPrice[i][epctMoney] > 0);
            }
        }

        if (m_ExteriorShopMap.find((DWORD)nID) != m_ExteriorShopMap.end())
        {
            EXTERIOR_DUP_LOG(szFileName, nLine, nID);
            goto Exit0;
        }
        m_ExteriorShopMap[(DWORD)nID] = Price;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_DEBUG, "Load %s failed at line %d.", EXTERIOR_SHOP_PRICE_TAB, nLine);
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

//------------------------------------------------------------------------
// Query accessors
//------------------------------------------------------------------------
// == LuaAddExterior grant target (v246 KExterior::AddExterior FUN_081f9498),
// money/coin-free. Computes expire like Delivery then KExteriorBox::Add.
BOOL KExterior::AddExterior(KPlayer* pPlayer, DWORD dwID, int nTimeType, int nPayType, int nBuySource)
{
    BOOL            bResult = false;
    KEXTERIOR_INFO* pInfo   = NULL;
    time_t          nExpire = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(nTimeType > ettInvalid && nTimeType < ettTotal);
    KGLOG_PROCESS_ERROR(nPayType  > epctInvalid && nPayType  < epctTotal);
    KGLOG_PROCESS_ERROR(nBuySource > ebsInvalid && nBuySource < ebsTotal);

    pInfo = GetExteriorInfo(dwID);
    KGLOG_PROCESS_ERROR(pInfo);
    // grant only defined price combos (v246: nPrice[timeType][payType] != -1)
    KGLOG_PROCESS_ERROR(pInfo->nPrice[nTimeType][nPayType] != -1);

    // Delivery expire rule: permanent=0, 7days=now+604800, limit=info.nLimitType
    if (nTimeType == ett7Days)
        nExpire = g_pSO3World->m_nCurrentTime + EXTERIOR_7DAYS_SECONDS;
    else if (nTimeType == ettLimit)
        nExpire = (time_t)pInfo->nLimitType;

    bResult = pPlayer->m_ExteriorBox.Add(dwID, nTimeType, nExpire);
Exit0:
    return bResult;
}

KEXTERIOR_INFO* KExterior::GetExteriorInfo(DWORD dwID)
{
    KEXTERIOR_INFO*             pResult = NULL;
    KEXTERIOR_INFO_MAP::iterator it     = m_ExteriorInfoMap.find(dwID);

    KGLOG_PROCESS_ERROR(it != m_ExteriorInfoMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

KEXTERIOR_SUIT_INFO* KExterior::GetExteriorSuitInfo(DWORD dwSuitID)
{
    KEXTERIOR_SUIT_INFO*         pResult = NULL;
    KEXTERIOR_SUIT_MAP::iterator it      = m_ExteriorSuitMap.find(dwSuitID);

    KGLOG_PROCESS_ERROR(it != m_ExteriorSuitMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

KEXTERIOR_SHOP_PRICE* KExterior::GetExteriorShopPrice(DWORD dwID)
{
    KEXTERIOR_SHOP_PRICE*        pResult = NULL;
    KEXTERIOR_SHOP_MAP::iterator it      = m_ExteriorShopMap.find(dwID);

    KGLOG_PROCESS_ERROR(it != m_ExteriorShopMap.end());
    pResult = &(it->second);
Exit0:
    return pResult;
}

DWORD KExterior::GetExteriorIndex(DWORD nSubType, DWORD nRepresentID, DWORD nColorID, DWORD nForceID)
{
    DWORD                         dwResult = 0;
    KEXTERIOR_INDEX_KEY           Key;
    KEXTERIOR_INDEX_MAP::iterator it;

    Key.nSubType     = nSubType;
    Key.nRepresentID = nRepresentID;
    Key.nColorID     = nColorID;
    Key.nForceID     = nForceID;

    it = m_ExteriorIndexMap.find(Key);
    KGLOG_PROCESS_ERROR(it != m_ExteriorIndexMap.end());
    dwResult = it->second;
Exit0:
    return dwResult;
}

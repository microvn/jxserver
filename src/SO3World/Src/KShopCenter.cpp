#include "stdafx.h"
#include <vector>
#include "Common/CRC32.h"
#include "KMath.h"
#include "KItem.h"
#include "StrMap.h"
#include "KCharacter.h"
#include "KSceneObject.h"
#include "KNpc.h"
#include "KSO3World.h"
#include "KShop.h"
#include "KItemLib.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

using namespace std;

#define NPC_SHOP_CONFIG_FILE          "NpcShopList.tab"
#define REPUTATION_REBATE_CONFIG_FILE "ReputationRebate.tab"

BOOL KShopCenter::Init()
{
    BOOL        bResult     = false;
    int         nRetCode    = false;

#ifdef _SERVER
    nRetCode = LoadNpcShopTemplates();
    KGLOG_PROCESS_ERROR(nRetCode);

    m_nNextRefreshTime = g_pSO3World->m_nGameLoop +
        (g_pSO3World->m_Settings.m_ConstList.nRefreshCycle << 4);
    m_dwShopID          = 0;
#endif

    m_ShopList.clear();
    m_ReputationRebateMap.clear();
    
    nRetCode = LoadReputationRebate();
    KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
#ifdef _SERVER
    if (!bResult)
    {
        m_NpcShopTemplateTable.clear();
    }
#endif
    return bResult;
}

void KShopCenter::UnInit()
{
    ClearShop();

    m_ReputationRebateMap.clear();

#ifdef _SERVER
    m_NpcShopTemplateTable.clear();
#endif
}

KShop* KShopCenter::GetShopInfo(DWORD dwShopID)
{
    KShop* pShop = NULL;
    KSHOP_LIST::iterator it;

    it = m_ShopList.find(dwShopID);
    KG_PROCESS_ERROR(it != m_ShopList.end());

    pShop = it->second;
Exit0:
    return pShop;
}

#ifdef _SERVER
void KShopCenter::Activate()
{
    if (g_pSO3World->m_nGameLoop >= m_nNextRefreshTime) 
    {
        DoRefresh();
        m_nNextRefreshTime = g_pSO3World->m_nGameLoop +
            (g_pSO3World->m_Settings.m_ConstList.nRefreshCycle << 4);
    }
}

KShop* KShopCenter::CreateShop(DWORD dwTemplateID)
{
    KShop*              pResult         = NULL;
    BOOL                bRetCode        = false;
    KNPC_SHOP_TEMPLATE* pShopTemplate   = NULL;
    int                 nPageIndex      = 0;
    int                 nPosIndex       = 0;
    KShop*              pNewShop        = NULL;
    KItem*              pItem           = NULL;
    KNPC_SHOP_TEMPLATE_TABLE::iterator it;
    pair<KSHOP_LIST::iterator, bool>   InsRet;

    it = m_NpcShopTemplateTable.find(dwTemplateID);
    KGLOG_PROCESS_ERROR(it != m_NpcShopTemplateTable.end());

    pShopTemplate = &it->second;
    KGLOG_PROCESS_ERROR(pShopTemplate);

    pNewShop = KMemory::New<KShop>();
    KGLOG_PROCESS_ERROR(pNewShop);
    bRetCode = pNewShop->Init();
    KGLOG_PROCESS_ERROR(bRetCode);

    pNewShop->m_dwShopID = m_dwShopID++;

    for (size_t i = 0; i < pShopTemplate->vecTemplateItems.size(); ++i)
    {
        KNPC_SHOP_TEMPLATE_ITEM* pTemplateItem  = NULL;
        KSHOP_ITEM*              pShopItem      = NULL;
        
        if (nPosIndex >= MAX_SHOP_PAGE_ITEM_COUNT)
        {
            nPosIndex = 0;
            ++nPageIndex;
        }

        if (nPageIndex >= MAX_SHOP_PAGES)
        {
            KGLogPrintf(KGLOG_ERR, "[Shop] Too many items in shop template %u !\n", dwTemplateID);
            goto Exit0;
        }
        
        pShopItem = &pNewShop->m_ShopPages[nPageIndex].ShopItems[nPosIndex++];
        
        memset(pShopItem, 0, sizeof(KSHOP_ITEM));

		pTemplateItem = &pShopTemplate->vecTemplateItems[i];

        pItem = g_pSO3World->m_ItemManager.GenerateItem(
            pTemplateItem->nType, pTemplateItem->nIndex, 
            g_pSO3World->m_nCurrentTime, ERROR_ID, pTemplateItem->dwRandomSeed
        );
        if (!pItem)
        {
            KGLogPrintf(
                KGLOG_ERR, 
                "[Shop] Shop template ID %u, invalid item (nType = %d, nIndex = %d, dwRandomSeed = %u) !\n", 
                dwTemplateID, pTemplateItem->nType, pTemplateItem->nIndex, pTemplateItem->dwRandomSeed
            );
            continue;
        }
        
        pShopItem->pItem = pItem;
        pItem = NULL;
        
        pShopItem->nCount               = pTemplateItem->nLimit;
        pShopItem->nItemTemplateIndex   = (int)i;

        if (pTemplateItem->nDurability != -1)
        {
            KGLOG_PROCESS_ERROR(pTemplateItem->nDurability >= 0);
            if (pShopItem->pItem->m_nMaxDurability > pTemplateItem->nDurability)
            {
                pShopItem->pItem->m_nCurrentDurability = pTemplateItem->nDurability;
            }
        }
    }
    
    memcpy(pNewShop->m_szShopName, pShopTemplate->szShopName, sizeof(pNewShop->m_szShopName));
    pNewShop->m_szShopName[sizeof(pNewShop->m_szShopName) - 1] = '\0';

    pNewShop->m_dwTemplateID      = dwTemplateID;
    pNewShop->m_nShopType         = eShopType_NPC;
    pNewShop->m_dwValidPage       = nPageIndex + 1;
    pNewShop->m_bCanRepair        = pShopTemplate->bCanRepair;
    pNewShop->m_bCoinShop         = pShopTemplate->bCoinShop;
    pNewShop->m_dwScriptID        = pShopTemplate->dwShopScriptID;
    pNewShop->m_dwRequireForceID  = pShopTemplate->dwRequireForceID;

    InsRet = m_ShopList.insert(make_pair(pNewShop->m_dwShopID, pNewShop));
    KGLOG_PROCESS_ERROR(InsRet.second);
    pResult = pNewShop;
    pNewShop = NULL;
Exit0:
    if (pItem)
    {
        g_pSO3World->m_ItemManager.FreeItem(pItem);
        pItem = NULL;
    }
    if (pNewShop)
    {
        pNewShop->UnInit();
        KMemory::Delete(pNewShop);
    }
    return pResult;
}

void KShopCenter::DestroyShop(DWORD dwShopID)
{
    KSHOP_LIST::iterator it = m_ShopList.find(dwShopID);
    if (it == m_ShopList.end())
        return;

    KShop* pShop = it->second;
    m_ShopList.erase(pShop->m_dwShopID);
    pShop->UnInit();
    KMemory::Delete(pShop);
}

void KShopCenter::DoRefresh()
{
    DWORD                   dwShopID    = 0;
    KShop*                  pShop       = NULL;
    KSHOP_LIST::iterator    it;

    for (it = m_ShopList.begin(); it != m_ShopList.end(); ++it) 
    {
        dwShopID = it->first;
        pShop = it->second;
        assert(pShop);

        if (pShop->m_nShopType != eShopType_NPC)
            continue;

        pShop->Refresh();
    }

    return;
}

int KShopCenter::LoadNpcShopTemplates()
{
    BOOL        bResult                     = false;
    int         nRetCode                    = false;
    int         nHeight                     = 0;
    ITabFile*   piTabFile                   = NULL;
    char        szNpcShopConfigFile[MAX_PATH];

    snprintf(szNpcShopConfigFile, sizeof(szNpcShopConfigFile), "%s/shop/%s", SETTING_DIR, NPC_SHOP_CONFIG_FILE);
    szNpcShopConfigFile[sizeof(szNpcShopConfigFile) - 1] = '\0';

    piTabFile = g_OpenTabFile(szNpcShopConfigFile);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();

    for (int nLine = 2; nLine <= nHeight; ++nLine) 
    {
        nRetCode = LoadLine(piTabFile, nLine);
        KGLOG_PROCESS_ERROR(nRetCode);
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_NpcShopTemplateTable.clear();
    }
    
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KShopCenter::LoadLine(ITabFile* pTabFile, int nLine)
{
    BOOL                                bResult = false;
    int                                 nRetCode = false;
    int                                 nTemplateID = 0;
    char                                szConfigFileName[MAX_PATH];
    char                                szConfigFilePath[MAX_PATH];
    char                                szScriptName[MAX_PATH];
    KNPC_SHOP_TEMPLATE_TABLE::iterator  it;
    pair<KNPC_SHOP_TEMPLATE_TABLE::iterator, bool> InsRet;
    KNPC_SHOP_TEMPLATE*                 pShopTemplate = NULL;

    KGLOG_PROCESS_ERROR(pTabFile);
    nRetCode = pTabFile->GetInteger(nLine, "ShopTemplateID", 0, &nTemplateID);
    KGLOG_PROCESS_ERROR(nRetCode);

    InsRet = m_NpcShopTemplateTable.insert(make_pair(nTemplateID, KNPC_SHOP_TEMPLATE()));
    KGLOG_PROCESS_ERROR(InsRet.second);
    it = InsRet.first;
    pShopTemplate = &it->second;

    nRetCode = pTabFile->GetString(nLine, "ShopName", "", pShopTemplate->szShopName, sizeof(pShopTemplate->szShopName));
    (void)nRetCode; /* target-tolerant field read */
    pShopTemplate->szShopName[sizeof(pShopTemplate->szShopName) - 1] = '\0';

    nRetCode = pTabFile->GetInteger(nLine, "CanRepair", 0, (int*)&pShopTemplate->bCanRepair);
    (void)nRetCode; /* target-tolerant field read */
    nRetCode = pTabFile->GetInteger(nLine, "RequireForceID", 0, (int*)&pShopTemplate->dwRequireForceID);
    (void)nRetCode; /* target-tolerant field read */

    nRetCode = pTabFile->GetString(nLine, "ScriptName", "", szScriptName, sizeof(szScriptName));
    szScriptName[sizeof(szScriptName) - 1] = '\0';
    if (nRetCode)
        pShopTemplate->dwShopScriptID = g_FileNameHash(szScriptName);

    nRetCode = pTabFile->GetString(nLine, "ConfigFile", "", szConfigFileName, sizeof(szConfigFileName));
    (void)nRetCode; /* target-tolerant field read */
    szConfigFileName[sizeof(szConfigFileName) - 1] = '\0';
    snprintf(szConfigFilePath, sizeof(szConfigFilePath), "%s/shop/%s", SETTING_DIR, szConfigFileName);
    szConfigFilePath[sizeof(szConfigFilePath) - 1] = '\0';

    KGLOG_PROCESS_ERROR(LoadNpcShopTemplateItems(pShopTemplate, szConfigFilePath));
    KGLOG_PROCESS_ERROR(CheckCoinShop(pShopTemplate));
    bResult = true;
Exit0:
    return bResult;
}
#endif

BOOL KShopCenter::LoadNpcShopTemplateItems(KNPC_SHOP_TEMPLATE* pShopTemplate, const char cszFileName[])
{
    BOOL         bResult            = false;
    int          nRetCode           = false;
    ITabFile*    pNPCShopConfigFile = NULL;
    int          nShopItemCount     = 0;
    BOOL         bCoinShop          = false;

    DECLARE_STRING_MAP_BEGIN(ITEM_TABLE_TYPE)
        REGISTER_STR_TO_VALUE("Other",          ittOther)
        REGISTER_STR_TO_VALUE("CustomWeapon",   ittCustWeapon)
        REGISTER_STR_TO_VALUE("CustomArmor",    ittCustArmor)
        REGISTER_STR_TO_VALUE("CustomTrinket",  ittCustTrinket)
    DECLARE_STRING_MAP_END(ITEM_TABLE_TYPE)

    assert(pShopTemplate);
    assert(cszFileName);

    pNPCShopConfigFile = g_OpenTabFile(cszFileName);
    KGLOG_PROCESS_ERROR(pNPCShopConfigFile);

    nShopItemCount = pNPCShopConfigFile->GetHeight() - 1;
    KGLOG_PROCESS_ERROR(nShopItemCount > 0 && nShopItemCount <= MAX_SHOP_PAGE_ITEM_COUNT * MAX_SHOP_PAGES);
    
    pShopTemplate->vecTemplateItems.resize(nShopItemCount);

    for (int i = 0; i < nShopItemCount; ++i)
    {
        int                         nTabType    = 0;
        KNPC_SHOP_TEMPLATE_ITEM*    pShopItem   = NULL;
        int                         nLine       = i + 2;
        char                        szValue[_NAME_LEN];
        
        pShopItem = &(pShopTemplate->vecTemplateItems[i]);

        nRetCode = pNPCShopConfigFile->GetString(nLine, "TabType", "-1", szValue, sizeof(szValue));
        (void)nRetCode; /*[endgame] tolerant*/

        MAP_STRING_LOCAL(ITEM_TABLE_TYPE, szValue, nTabType);
        KGLOG_PROCESS_ERROR(nTabType != -1);

        pShopItem->nType = nTabType;

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ItemIndex", -1, &pShopItem->nIndex);          
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(pShopItem->nIndex != -1);
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "RandSeed", -1, (int*)&pShopItem->dwRandomSeed);               
        (void)nRetCode; /*[endgame] tolerant*/
        KGLOG_PROCESS_ERROR(pShopItem->dwRandomSeed != -1);
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "CountLimit", -1, &pShopItem->nLimit);     
        (void)nRetCode; /*[endgame] tolerant*/
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Durability", -1, &pShopItem->nDurability);
        (void)nRetCode; /*[endgame] tolerant*/
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ReputeLevel", 0, &pShopItem->nReputeLevel);     
        (void)nRetCode; /*[endgame] tolerant*/
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Price", -1, &pShopItem->nPrice);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Coin", 0, &pShopItem->nCoin);
        KGLOG_PROCESS_ERROR(nRetCode);

        KGLOG_PROCESS_ERROR(pShopItem->nPrice > 0 || pShopItem->nCoin > 0);
        if (pShopItem->nCoin > 0)
            bCoinShop = true;

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Prestige", -1, &pShopItem->nPrestige);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Contribution", -1, &pShopItem->nContribution);
        (void)nRetCode; /*[endgame] tolerant*/

        // v2.5 NEW currency prices (KCurrency types 2..5)
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "Justice", 0, &pShopItem->nJustice);
        (void)nRetCode; /*[endgame] tolerant*/
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ExamPrint", 0, &pShopItem->nExamPrint);
        (void)nRetCode; /*[endgame] tolerant*/
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ArenaAward", 0, &pShopItem->nArenaAward);
        (void)nRetCode; /*[endgame] tolerant*/
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ActivityAward", 0, &pShopItem->nActivityAward);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "RequireAchievementRecord", -1, &pShopItem->nRequireAchievementRecord);     
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "AchievementPoint", -1, &pShopItem->nAchievementPoint);     
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "TongReputation", 0, &pShopItem->nTongReputation);
        (void)nRetCode;

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ItemType1", 0, (int*)&pShopItem->dwTabType);     
        (void)nRetCode; /*[endgame] tolerant*/
        
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ItemIndex1", 0, (int*)&pShopItem->dwIndex);     
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "ItemCount1", 0, &pShopItem->nRequireAmount);     
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "MentorValue", 0, &pShopItem->nMentorValue);
        (void)nRetCode;
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "RequireTitle", 0, &pShopItem->nRequireTitle);
        (void)nRetCode;
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "RequireCorpsValue", 0, &pShopItem->nRequireCorpsValue);
        (void)nRetCode;
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "MaskCorpsNeedToCheck", 0, (int*)&pShopItem->dwMaskCorpsNeedToCheck);
        (void)nRetCode;
        nRetCode = pNPCShopConfigFile->GetInteger(nLine, "CanReturn", 0, (int*)&pShopItem->bCanReturn);
        (void)nRetCode;

        if (pShopItem->nContribution > 0 || pShopItem->nExamPrint > 0)
            SetMentorItemScore(pShopItem->dwTabType, pShopItem->dwIndex,
                pShopItem->nContribution, pShopItem->nExamPrint);

        KGLOG_PROCESS_ERROR(pShopItem->nRequireCorpsValue == 0 || pShopItem->dwMaskCorpsNeedToCheck != 0);
    }

    pShopTemplate->bCoinShop = bCoinShop;
    bResult = true;
Exit0:
    KG_COM_RELEASE(pNPCShopConfigFile);
    return bResult;
}


KNPC_SHOP_TEMPLATE_ITEM* KShopCenter::GetShopTemplateItem(DWORD dwShopTemplateID, int nItemTemplateIndex)
{
    KNPC_SHOP_TEMPLATE_ITEM*    pResult = NULL;
    KNPC_SHOP_TEMPLATE*         pShopTemplate   = NULL;
    KNPC_SHOP_TEMPLATE_TABLE::iterator it;

    it = m_NpcShopTemplateTable.find(dwShopTemplateID);
    KGLOG_PROCESS_ERROR(it != m_NpcShopTemplateTable.end());

    pShopTemplate = &it->second;
    KGLOG_PROCESS_ERROR(pShopTemplate);

    KGLOG_PROCESS_ERROR(nItemTemplateIndex < (int)pShopTemplate->vecTemplateItems.size());
    
    pResult = &pShopTemplate->vecTemplateItems[nItemTemplateIndex];
Exit0:
    return pResult;
}

int KShopCenter::GetMentorItemScore(DWORD dwTabType, DWORD dwIndex)
{
    int nResult = 0;
    int64_t llKey = MAKE_INT64(dwTabType, dwIndex);
    ITEM_CONTRIBUTION_MAP::iterator it = m_ItemContributionMap.find(llKey);

    if (it != m_ItemContributionMap.end())
        nResult = it->second;

    return nResult;
}

void KShopCenter::SetMentorItemScore(DWORD dwTabType, DWORD dwIndex, int nContribution, int nExamPrint)
{
    int nScore = nContribution / 100 + nExamPrint;
    int64_t llKey = MAKE_INT64(dwTabType, dwIndex);
    pair<ITEM_CONTRIBUTION_MAP::iterator, bool> InsertResult;

    if (nScore == 0)
        return;

    InsertResult = m_ItemContributionMap.insert(ITEM_CONTRIBUTION_MAP::value_type(llKey, nScore));
    if (!InsertResult.second && InsertResult.first->second < nScore)
        InsertResult.first->second = nScore;
}

BOOL KShopCenter::CheckCoinShop(KNPC_SHOP_TEMPLATE* pShopTemplate)
{
    KGLOG_PROCESS_ERROR(pShopTemplate);

    if (pShopTemplate->bCoinShop)
    {
        for (size_t i = 0; i < pShopTemplate->vecTemplateItems.size(); ++i)
        {
            KNPC_SHOP_TEMPLATE_ITEM* pItem = &pShopTemplate->vecTemplateItems[i];

            KGLOG_PROCESS_ERROR(pItem->nCoin >= 1);
            KGLOG_PROCESS_ERROR(pItem->nPrice == 0);
            KGLOG_PROCESS_ERROR(pItem->nContribution == 0);
            KGLOG_PROCESS_ERROR(pItem->nPrestige == 0);
            KGLOG_PROCESS_ERROR(pItem->nRequireAchievementRecord == 0);
            KGLOG_PROCESS_ERROR(pItem->nAchievementPoint == 0);
            KGLOG_PROCESS_ERROR(pItem->dwTabType == 0);
            KGLOG_PROCESS_ERROR(pItem->nRequireAmount == 0);
            KGLOG_PROCESS_ERROR(pItem->nMentorValue == 0);
        }
    }

    return true;
Exit0:
    return false;
}

BOOL KShopCenter::OnBuyCoinShopItem(KPlayer* pPlayer, int nResult, DWORD dwTabType, DWORD dwIndex,
    int nRandomSeed, int nCount, int nCoinCost)
{
    BOOL    bResult = false;
    BOOL    bRetCode = false;
    KItem*  pItem = NULL;
    int     nNotifyCode = 7;
    int     nRefundCoin = 0;

    KGLOG_PROCESS_ERROR(pPlayer);

    if (nResult == 0)
    {
        nRefundCoin = (int)((uint32_t)nCoinCost * (uint32_t)nCount);
        KGLOG_PROCESS_ERROR(nRefundCoin >= 0);

        bRetCode = pPlayer->AddCoin(nRefundCoin);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        pItem = g_pSO3World->m_ItemManager.GenerateItem(
            dwTabType, dwIndex, g_pSO3World->m_nCurrentTime, ERROR_ID, nRandomSeed
        );
        KGLOG_PROCESS_ERROR(pItem);

        if (pItem->IsStackable())
        {
            bRetCode = pItem->SetStackNum(nCount);
            KGLOG_PROCESS_ERROR(bRetCode);
            KGLOG_PROCESS_ERROR(nRandomSeed == 0);
        }
        else
        {
            KGLOG_PROCESS_ERROR(nCount == 1);
            pItem->m_GenParam.dwRandSeed = (DWORD)nRandomSeed;
        }

        bRetCode = pPlayer->m_ItemList.AddItem(pItem);
        KGLOG_PROCESS_ERROR(bRetCode == aircSuccess);
        pItem = NULL;
        nNotifyCode = 2;
    }

    bResult = true;
Exit0:
    if (pItem)
        g_pSO3World->m_ItemManager.FreeItem(pItem);

    g_PlayerServer.DoMessageNotify(pPlayer->m_nConnIndex, 15, nNotifyCode);
    return bResult;
}

#ifdef _CLIENT
BOOL KShopCenter::UpDateShopInfo(DWORD dwShopTemplateID, DWORD dwShopID, int nShopType, DWORD dwValidPage, BOOL bCanRepair)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KShop*      pShop       = NULL;
    KSHOP_LIST::iterator it;
    KNPC_SHOP_TEMPLATE_TABLE::iterator itTemplate;

    it = m_ShopList.find(dwShopID);
    if (it != m_ShopList.end())
    {
        pShop = it->second;
        pShop->m_nShopType    = nShopType;
        pShop->m_dwValidPage  = dwValidPage;        
        pShop->m_bCanRepair   = bCanRepair;
    }
    else
    {
        KShop* pNewShop = NULL;
        pair<KSHOP_LIST::iterator, bool> IntResult;

        pNewShop = KMemory::New<KShop>();
        KGLOG_PROCESS_ERROR(pNewShop);
        KGLOG_PROCESS_ERROR(pNewShop->Init());
        pNewShop->m_dwShopID       = dwShopID;
        pNewShop->m_nShopType      = nShopType;
        pNewShop->m_dwValidPage    = dwValidPage;
        pNewShop->m_bCanRepair     = bCanRepair;
        pNewShop->m_dwTemplateID   = dwShopTemplateID;

        IntResult = m_ShopList.insert(KSHOP_LIST::value_type(dwShopID, pNewShop));
        KGLOG_PROCESS_ERROR(IntResult.second);
        pShop = IntResult.first->second;
    }
    
    itTemplate = m_NpcShopTemplateTable.find(dwShopTemplateID);
    
    if (itTemplate == m_NpcShopTemplateTable.end())
    {
        bRetCode = LoadShopTemplate(dwShopTemplateID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopCenter::LoadShopTemplate(DWORD dwShopTemplateID)
{
    BOOL        bResult                     = false;
    int         nRetCode                    = false;
    int         nHeight                     = 0;
    ITabFile*   piTabFile                   = NULL;
    char        szNpcShopConfigFile[MAX_PATH];
    char        szTemplateFileName[MAX_PATH];
    char        szConfigFilePath[MAX_PATH];
    KNPC_SHOP_TEMPLATE_TABLE::iterator it;

    snprintf(szNpcShopConfigFile, sizeof(szNpcShopConfigFile), "%s/shop/%s", SETTING_DIR, NPC_SHOP_CONFIG_FILE);
    szNpcShopConfigFile[sizeof(szNpcShopConfigFile) - 1] = '\0';

    piTabFile = g_OpenTabFile(szNpcShopConfigFile);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();

    for (int nLine = 2; nLine <= nHeight; ++nLine) 
    {
        int                                     nTemplateID         = 0;
        KNPC_SHOP_TEMPLATE*                     pNpcShopTemplate    = NULL;
        pair<KNPC_SHOP_TEMPLATE_TABLE::iterator, bool>    InsRet;
        
        nRetCode = piTabFile->GetInteger(nLine, "ShopTemplateID", 0, &nTemplateID);
        (void)nRetCode; /*[endgame] tolerant*/

        if (nTemplateID != (int)dwShopTemplateID)
        {
            continue;
        }
        
        InsRet = m_NpcShopTemplateTable.insert(make_pair(nTemplateID, KNPC_SHOP_TEMPLATE()));
        KGLOG_PROCESS_ERROR(InsRet.second);
        
        pNpcShopTemplate = &(InsRet.first->second);

        nRetCode = piTabFile->GetString(nLine, "ShopName", "", pNpcShopTemplate->szShopName, sizeof(pNpcShopTemplate->szShopName));
        (void)nRetCode; /*[endgame] tolerant*/
        pNpcShopTemplate->szShopName[sizeof(pNpcShopTemplate->szShopName) - 1] = '\0';

        nRetCode = piTabFile->GetInteger(nLine, "CanRepair", 0, (int*)&pNpcShopTemplate->bCanRepair);    
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = piTabFile->GetString(nLine, "ConfigFile", "", szTemplateFileName, sizeof(szTemplateFileName));
        (void)nRetCode; /*[endgame] tolerant*/
        szTemplateFileName[sizeof(szTemplateFileName) - 1] = '\0';

        snprintf(szConfigFilePath, sizeof(szConfigFilePath), "%s/shop/%s", SETTING_DIR, szTemplateFileName);
        szConfigFilePath[sizeof(szConfigFilePath) - 1] = '\0';

        nRetCode = LoadNpcShopTemplateItems(pNpcShopTemplate, szConfigFilePath);
        if (!nRetCode)
        {
            KGLogPrintf(KGLOG_ERR, "Load shop %s failed!", szTemplateFileName);
            goto Exit0;
        }
		break;
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_NpcShopTemplateTable.clear();
    }
    
    KG_COM_RELEASE(piTabFile);
    return bResult;
}
#endif

BOOL KShopCenter::LoadReputationRebate()
{
    BOOL                bResult                             = false;
    int                 nRetCode                            = 0;
    DWORD               dwForceID                           = 0;
    int                 nReputeLevel                        = 0;
    ITabFile*           piTabFile                           = NULL;
    int                 nHeight                             = 0;
    char                szReputationRebateTab[MAX_PATH];
    REPUTATION_REBATE   ReputationRebate;


    snprintf(szReputationRebateTab, sizeof(szReputationRebateTab), "%s/shop/%s", SETTING_DIR, REPUTATION_REBATE_CONFIG_FILE);
    szReputationRebateTab[sizeof(szReputationRebateTab) - 1] = '\0';
    piTabFile = g_OpenTabFile(szReputationRebateTab);
    
    if (!piTabFile) 
    {
        KGLogPrintf(KGLOG_DEBUG, "[shop] can not find Reputation rebate config file %s\n", szReputationRebateTab);
        goto Exit0;
    }
    
    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    for (int nLine = 2; nLine <= nHeight; ++nLine)
    {
        uint64_t uKey = 0;
        REPUTATION_REBATE_MAP::iterator it;

        nRetCode = piTabFile->GetInteger(nLine, "ForceID", 0, (int*)&dwForceID);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = piTabFile->GetInteger(nLine, "ReputeLevel", 0, &nReputeLevel);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = piTabFile->GetInteger(nLine, "Rebate", 0, &ReputationRebate.nRebate);
        (void)nRetCode; /*[endgame] tolerant*/

        nRetCode = piTabFile->GetInteger(nLine, "MaxRebate", 0, &ReputationRebate.nMaxRebate);
        (void)nRetCode; /*[endgame] tolerant*/

        uKey = MAKE_INT64(dwForceID, nReputeLevel);
        
        it = m_ReputationRebateMap.find(uKey);
        if (it != m_ReputationRebateMap.end())
        {
            KGLogPrintf(
              KGLOG_ERR, "[Shop] config file %s two line have same ForceID and ReputeLevel", szReputationRebateTab   
            );
            goto Exit0;
        }
        
        m_ReputationRebateMap[uKey] = ReputationRebate;
    }
    
    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

int KShopCenter::GetReputationRebate(DWORD dwForceID, int nReputeLevel)
{
    int         nResult = KILO_NUM;
    uint64_t    uKey    = 0;
    REPUTATION_REBATE_MAP::iterator  it;

    uKey    = MAKE_INT64(dwForceID, nReputeLevel);
    it      = m_ReputationRebateMap.find(uKey);
    KG_PROCESS_ERROR(it != m_ReputationRebateMap.end());
    
    nResult = it->second.nRebate;

Exit0:
    return nResult;
}

int KShopCenter::GetMaxRebate(DWORD dwForceID, int nReputeLevel)
{
    int         nResult = KILO_NUM;
    uint64_t    uKey = 0; 
    REPUTATION_REBATE_MAP::iterator it;

    uKey = MAKE_INT64(dwForceID, nReputeLevel);
    it = m_ReputationRebateMap.find(uKey);
    KG_PROCESS_ERROR(it != m_ReputationRebateMap.end());

    nResult = it->second.nMaxRebate;

Exit0:
    return nResult;
}

void KShopCenter::ClearShop()
{
    for (KSHOP_LIST::iterator ShopIterator = m_ShopList.begin(); ShopIterator != m_ShopList.end(); ++ShopIterator)
    {
        KShop* pShop = ShopIterator->second;
        pShop->UnInit();
        KMemory::Delete(pShop);
    }

    m_ShopList.clear();
}

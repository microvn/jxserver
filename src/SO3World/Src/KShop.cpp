////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KShop.cpp
//  Version     : 1.0
//  Creator     : Zhao Chunfeng
//  Create Date : 2005-11-17 14:50:41
//  Comment     : 
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "KShop.h"
#include "KPlayer.h"
#include "KNpc.h"
#include "KPlayerServer.h"
#include "KSO3World.h"
// target KShop::BuyCoinShopItem @0x081455cc calls KRelayClient::DoCoinShopBuyItemRequest
// @0x080c7d28; g_RelayClient is declared as `extern KRelayClient g_RelayClient;` at the end
// of KRelayClient.h inside #ifdef _SERVER.
#include "KRelayClient.h"

BOOL KShop::Init()
{
    m_dwTemplateID    = 0;
    m_dwRequireForceID = 0;
    m_dwShopID        = 0;
    m_nShopType       = eShopType_Invalid;
    m_dwValidPage     = 0;
    m_bCanRepair      = false;
    m_bCoinShop       = false;
    m_dwNpcID         = 0;
    m_pNpc            = NULL;
    m_dwScriptID      = 0;
    memset(m_ShopPages, 0, sizeof(m_ShopPages));

    return true;
}

void KShop::UnInit()
{
    for (int nPageIndex = 0; nPageIndex < MAX_SHOP_PAGES; ++nPageIndex)
    {
        for (int nPosIndex = 0; nPosIndex < MAX_SHOP_PAGE_ITEM_COUNT; ++nPosIndex)
        {
            KSHOP_ITEM* pShopItem = &m_ShopPages[nPageIndex].ShopItems[nPosIndex];
            if (pShopItem->pItem)
            {
                g_pSO3World->m_ItemManager.FreeItem(pShopItem->pItem);
                pShopItem->pItem = NULL;
            }

            pShopItem->nCount = 0;
            pShopItem->nItemTemplateIndex = 0;
        }
    }

    if (m_pNpc)
    {
        m_pNpc = NULL;
        m_dwNpcID = 0;
    }
}

#ifdef _CLIENT
BOOL KShop::UpdateItem(DWORD dwPageIndex, DWORD dwPosIndex, const KSHOP_ITEM &crShopItem)
{
    BOOL                bResult     = false;
    KSHOP_ITEM*         pShopItem   = NULL;
  
    KGLOG_PROCESS_ERROR(dwPageIndex < m_dwValidPage);
    KGLOG_PROCESS_ERROR(dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT);

    pShopItem = &m_ShopPages[dwPageIndex].ShopItems[dwPosIndex];
    *pShopItem = crShopItem;

    bResult = true;
Exit0:
    return bResult;
}
#endif

SHOP_SYSTEM_RESPOND_CODE KShop::CanBuyItem(KPlayer* pBuyer, KNpc* pSeller, KSHOP_BUY_ITEM_PARAM& rParam)
{
    SHOP_SYSTEM_RESPOND_CODE    nResult         = ssrcBuyFailed;
    int                         nRetCode        = 0;
    KSHOP_ITEM*                 pShopItem       = NULL;
    int                         nPlayerMoney    = 0;
    int64_t                     llBuyCost       = 0;
    KNPC_SHOP_TEMPLATE_ITEM*    pItemTemplate   = NULL;

    KGLOG_PROCESS_ERROR(pBuyer);
    KGLOG_PROCESS_ERROR(pSeller);
    KGLOG_PROCESS_ERROR(rParam.dwPageIndex < m_dwValidPage);
    KGLOG_PROCESS_ERROR(rParam.dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT);
    KGLOG_PROCESS_ERROR(rParam.nCount > 0);

    nRetCode = g_InRange(pBuyer, pSeller, COMMON_PLAYER_OPERATION_DISTANCE);
    KG_PROCESS_ERROR_RET_CODE(nRetCode, ssrcTooFarAway);

    nRetCode = pBuyer->GetNpcRelation(pSeller);
    KGLOG_PROCESS_ERROR(!(nRetCode & sortEnemy));
    
    pShopItem = &m_ShopPages[rParam.dwPageIndex].ShopItems[rParam.dwPosIndex];
    KGLOG_PROCESS_ERROR(pShopItem->pItem);
    KGLOG_PROCESS_ERROR(pShopItem->pItem->m_dwID == rParam.dwItemID);
    KGLOG_PROCESS_ERROR(pShopItem->pItem->m_Common.bStack || rParam.nCount == 1);

    pItemTemplate = g_pSO3World->m_ShopCenter.GetShopTemplateItem(m_dwTemplateID, pShopItem->nItemTemplateIndex);
    KGLOG_PROCESS_ERROR(pItemTemplate);
    KGLOG_PROCESS_ERROR(pItemTemplate->nType == (int)pShopItem->pItem->m_GenParam.dwTabType);
    KGLOG_PROCESS_ERROR(pItemTemplate->nIndex == (int)pShopItem->pItem->m_GenParam.dwIndex);

    nRetCode = pBuyer->m_ReputeList.GetReputeLevel(pSeller->m_dwForceID);
    KG_PROCESS_ERROR_RET_CODE(nRetCode >= pItemTemplate->nReputeLevel, ssrcNotEnoughRepate);

    /* PORT-UNKNOWN_REQUIRED[CALLER] owner=KPlayer Corps/Arena; target=0x08144898 -> KPlayer::CheckCorpsValue@0x08385642; phase=PRE_BUILD.
       Target rejects failed CheckCorpsValue(nRequireCorpsValue, dwMaskCorpsNeedToCheck) with response 42.
       The current KPlayer owner lacks this declared callee, so this standalone root cannot emulate or bypass it. */
    
    nRetCode = pShopItem->pItem->GetMaxStackNum();
    KG_PROCESS_ERROR_RET_CODE(rParam.nCount <= nRetCode, ssrcBuyFailed);

    llBuyCost = GetPlayerBuyCost(pBuyer, pItemTemplate->nPrice, rParam.nCount);
    KGLOG_PROCESS_ERROR(llBuyCost == rParam.nCost);

    nPlayerMoney = pBuyer->m_ItemList.GetMoney();
    KG_PROCESS_ERROR_RET_CODE(nPlayerMoney >= llBuyCost, ssrcNotEnoughMoney);
    
    if (pItemTemplate->nPrestige > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(
            pBuyer->m_nCurrentPrestige >= pItemTemplate->nPrestige * rParam.nCount, ssrcNotEnoughPrestige
        );
    }
    
    if (pItemTemplate->nContribution > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(pBuyer->m_nContribution >= pItemTemplate->nContribution * rParam.nCount, ssrcNotEnoughContribution);
    }

    // v2.5: yuanbao + the 4 new (non-legacy) currency prices. Afford-check is server-side (anti-hack).
    if (pItemTemplate->nCoin > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(pBuyer->m_nCoin >= pItemTemplate->nCoin * rParam.nCount, ssrcNotEnoughCoin);
    }
    {
        int nCurIdx = 0, nCurCost = 0;
        // {KCurrency type, per-unit cost, respond-code}
        for (nCurIdx = 0; nCurIdx < 4; nCurIdx++)
        {
            int       nType = 2 + nCurIdx;   // cbtJustice=2, cbtExamPrint=3, cbtArenaAward=4, cbtActivityAward=5
            KCurrency* pCur = NULL;
            switch (nCurIdx)
            {
            case 0: nCurCost = pItemTemplate->nJustice;       break;
            case 1: nCurCost = pItemTemplate->nExamPrint;     break;
            case 2: nCurCost = pItemTemplate->nArenaAward;    break;
            default:nCurCost = pItemTemplate->nActivityAward; break;
            }
            if (nCurCost <= 0)
                continue;
            pCur = pBuyer->m_CurrencyList.GetCurrency(nType);
            KGLOG_PROCESS_ERROR(pCur);
            KG_PROCESS_ERROR_RET_CODE(pCur->GetValue() >= nCurCost * rParam.nCount,
                (SHOP_SYSTEM_RESPOND_CODE)(ssrcNotEnoughJustice + nCurIdx));
        }
    }

    if (pItemTemplate->nRequireAchievementRecord > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(pBuyer->m_Achievement.m_nRecord >= pItemTemplate->nRequireAchievementRecord, ssrcAchievementRecordError);
    }

    if (pItemTemplate->nAchievementPoint > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(pBuyer->m_Achievement.m_nPoint >= pItemTemplate->nAchievementPoint * rParam.nCount, ssrcNotEnoughAchievementPoint);
    }

    if (pItemTemplate->nMentorValue > 0)
    {
        KG_PROCESS_ERROR_RET_CODE(
            pBuyer->m_nUsableMentorValue >= pItemTemplate->nMentorValue * rParam.nCount,
            (SHOP_SYSTEM_RESPOND_CODE)40
        );
    }

    /* PORT-UNKNOWN_REQUIRED[STATE] owner=KPlayer title state; target=0x08144898 field player+0xb6d0;
       phase=PRE_BUILD. Target rejects nRequireTitle with response 40; no target-backed title-state accessor exists in this closure. */

    if (pItemTemplate->nLimit != -1)
    {
        KGLOG_PROCESS_ERROR(pShopItem->nCount >= rParam.nCount);
        /* PORT-UNKNOWN_REQUIRED[STATE] owner=KPlayer timer state; target=0x08144898 -> KCDTimerList::CheckTimer;
           phase=PRE_BUILD. The target additionally enforces the item time-limit timer and template nLimit threshold. */
    }
    
    if (pItemTemplate->dwTabType > 0 && pItemTemplate->dwIndex > 0 && pItemTemplate->nRequireAmount > 0)
    {
        DWORD dwAmount = pBuyer->m_ItemList.GetItemAmountInEquipAndPackage(pItemTemplate->dwTabType, pItemTemplate->dwIndex);
        KG_PROCESS_ERROR_RET_CODE((int)dwAmount >= pItemTemplate->nRequireAmount * rParam.nCount, ssrcNotEnoughItem);
    }
    
    if (rParam.nCount > 1)
    {
        int nOldStackNum = pShopItem->pItem->GetStackNum();
        pShopItem->pItem->SetStackNum(rParam.nCount);
        nRetCode = pBuyer->m_ItemList.CanAddItem(pShopItem->pItem);
        pShopItem->pItem->SetStackNum(nOldStackNum);
    }   
    else
    {
        nRetCode = pBuyer->m_ItemList.CanAddItem(pShopItem->pItem);
    }

    KG_PROCESS_ERROR_RET_CODE(nRetCode != aircNotEnoughFreeRoom, ssrcBagFull);
    KG_PROCESS_ERROR_RET_CODE(nRetCode != aircItemAmountLimited, ssrcItemExistLimit);
    KG_PROCESS_ERROR(nRetCode == aircSuccess);
	
    nResult = ssrcBuySuccess;
Exit0:
    return nResult;
}

int64_t KShop::GetPlayerBuyCost(KPlayer* pPlayer, int nTemplateItemPrice, int nCount)
{
    int64_t     nResult     = -1;
    int         nPrice      = 0;
    KSHOP_ITEM* pShopItem   = NULL;
    
    KGLOG_PROCESS_ERROR(nCount > 0);
    
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_dwNpcID == m_pNpc->m_dwID);
    
    switch (m_nShopType)
    {
    case eShopType_NPC:
        {
            int nReputeLevel    = pPlayer->m_ReputeList.GetReputeLevel(m_pNpc->m_dwForceID);
            int nRebate         = g_pSO3World->m_ShopCenter.GetReputationRebate(m_pNpc->m_dwForceID, nReputeLevel);
            int nMaxRebate      = g_pSO3World->m_ShopCenter.GetMaxRebate(m_pNpc->m_dwForceID, nReputeLevel);
            long long llShopItemPrice = nTemplateItemPrice; // ��ֹ���

            nRebate = max(nRebate, nMaxRebate);

            nPrice = (int)(llShopItemPrice * nRebate / KILO_NUM);
            KG_ASSERT_EXIT(nPrice >= 0);

            nPrice = max(nPrice, 1);
        }
        break;
    case eShopType_PLAYER:       
        nPrice = nTemplateItemPrice;
        break;
    default:
        break;
    }

    nResult = (int64_t)nPrice * nCount;
Exit0:
    return nResult;
}

int KShop::GetPlayerSellPrice(KPlayer* pPlayer, DWORD dwBox, DWORD dwX)
{
    int         nResult             = -1;
    long long   llPrice             = 0;
    KItem*      pItem               = NULL;
    int         nCurrentDurability  = 0;

    assert(pPlayer);

    pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
    KGLOG_PROCESS_ERROR(pItem);

    switch (m_nShopType)
    {
    case eShopType_NPC:
        llPrice = pItem->GetPrice();
        KGLOG_PROCESS_ERROR(llPrice >= 0);
        // �����Ʒ���;ö�,Ҫ�۳���Ӧ�ķ���(������������ò��ܸ�������������, Ϊ�˷�ֹˢǮ)
        
        nCurrentDurability = pItem->GetCurrentDurability();
        if (nCurrentDurability != -1 && pItem->m_nMaxDurability > 0)
        {
            llPrice = (llPrice * nCurrentDurability / pItem->m_nMaxDurability);
        }

        break;
    case eShopType_PLAYER:
        // TODO
        break;
    default:
        break;
    }

    nResult = (int)llPrice;
Exit0:
    return nResult;
}

int KShop::GetPlayerRepairPrice(KPlayer* pPlayer, DWORD dwBox, DWORD dwX)
{
    int     nResult     = -1;
    int     nPrice      = 0;
    KItem*  pItem       = NULL;
    BOOL    bRepairable = false;

    KGLOG_PROCESS_ERROR(pPlayer);

    pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
    KGLOG_PROCESS_ERROR(pItem);

    bRepairable = pItem->IsRepairable();
    KG_PROCESS_ERROR(bRepairable);

    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_dwNpcID == m_pNpc->m_dwID);
    
    switch (m_nShopType)
    {
    case eShopType_NPC:
        {
            int         nReputeLevel         = 0;
            int         nRebate              = 0;
            int         nMaxRebate           = 0;
            long long   llPricePerDurability = 0;

            nReputeLevel    = pPlayer->m_ReputeList.GetReputeLevel(m_pNpc->m_dwForceID);
            nRebate         = g_pSO3World->m_ShopCenter.GetReputationRebate(m_pNpc->m_dwForceID, nReputeLevel);
            nMaxRebate      = g_pSO3World->m_ShopCenter.GetMaxRebate(m_pNpc->m_dwForceID, nReputeLevel);
            
            nRebate = max(nRebate, nMaxRebate);
            
            if (pItem->m_Common.nLevel <= 70)
            {
                llPricePerDurability = (588 * pItem->m_Common.nLevel * pItem->m_Common.nLevel - 1765 * pItem->m_Common.nLevel + 19190) / 20000;
            }
            else
            {
                llPricePerDurability = (1385 + 2 * (pItem->m_Common.nLevel - 70)) / 10;
            }

            nPrice = (int)((pItem->m_nMaxDurability - pItem->m_nCurrentDurability) * llPricePerDurability * nRebate / KILO_NUM);
            KGLOG_PROCESS_ERROR(nPrice >= 0); 

            nPrice = max(nPrice, 1);
        }

        break;
    case eShopType_PLAYER:
        KG_ASSERT_EXIT(false);
        break;
    default:
        assert(false);
        break;
    }

    nResult = nPrice;
Exit0:
    return nResult;
}

int KShop::GetPlayerAllRepairPrice(KPlayer* pPlayer)
{
    int nResult = 0;

    KGLOG_PROCESS_ERROR(pPlayer);

    for (DWORD dwBox = 0; dwBox < ibTotal; ++dwBox)
    {
        int nBoxType = pPlayer->m_ItemList.GetBoxType(dwBox);
        if (nBoxType != ivtEquipment && nBoxType != ivtPackage)
            continue;

        DWORD dwBoxSize = pPlayer->m_ItemList.GetBoxSize(dwBox);
        for (DWORD dwX = 0; dwX < dwBoxSize; ++dwX)
        {
            KItem* pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
            if (!pItem || !pItem->IsRepairable())
                continue;

            int nPrice = GetPlayerRepairPrice(pPlayer, dwBox, dwX);
            KGLOG_PROCESS_ERROR(nPrice >= 0);
            KGLOG_PROCESS_ERROR(nResult <= INT_MAX - nPrice);
            nResult += nPrice;
        }
    }

    return nResult;
Exit0:
    return -1;
}

// Ported 1:1 from target KShop::CanReturnItem @0x081423a6 (DWARF + disassembly).
// Model note: target uses IItem/KItemProperty; the 2010 tree uses KItem/KItemInfo, so
// pItem->GetProperty()->pItemInfo collapses to pItem->GetItemInfo() and the vtable calls
// map to the named accessors (vt+0x10 CanStack -> IsStackable, vt+0x4c
// GetCurrentDurability, vt+0x1c SetStackNum, vt+0x30 GetMaxStackNum). Verified against
// KGItem's vtable in jx3_dwarf/libSO3ItemHouseD.so.
SHOP_SYSTEM_RESPOND_CODE KShop::CanReturnItem(KPlayer* pPlayer, DWORD dwBox, DWORD dwX)
{
    SHOP_SYSTEM_RESPOND_CODE    nResult     = ssrcReturnFailed;
    BOOL                        bRetCode    = false;
    BOOL                        bStackable  = false;
    int                         nBoxType    = 0;
    DWORD                       dwShopTemplateID = 0;
    int                         nShopItemIndex   = 0;
    int                         nMaxDurability   = 0;
    int                         nCurDurability   = 0;
    KItem*                      pItem       = NULL;
    KItemInfo*                  pItemInfo   = NULL;
    KNPC_SHOP_TEMPLATE_ITEM*    pShopItem   = NULL;
    KItem*                      pAddItem    = NULL;
    KItemInfo*                  pAddItemInfo = NULL;
    int                         nCostList[6];
    int                         nCodeList[6];
    int                         i           = 0;

    KG_PROCESS_ERROR(g_pSO3World->m_bReturnItemFlag);
    KGLOG_PROCESS_ERROR(pPlayer);
    KG_PROCESS_ERROR(!pPlayer->m_bFightState);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);

    nBoxType = pPlayer->m_ItemList.GetBoxType(dwBox);
    KG_PROCESS_ERROR(nBoxType == ivtEquipment || nBoxType == ivtPackage);

    pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
    KG_PROCESS_ERROR(pItem);

    // target 0x08142530: KG_PROCESS_ERROR(!pItem->CanStack())
    bStackable = pItem->IsStackable();
    KG_PROCESS_ERROR(!bStackable);

    pItemInfo = pItem->GetItemInfo();
    KGLOG_PROCESS_ERROR(pItemInfo);

    // Refund is prorated by durability: KGItemInfo+0x20 is the
    // {nMaxDurability, nPackageSize} union, not a stack count.
    nCurDurability = pItem->GetCurrentDurability();
    nMaxDurability = pItemInfo->nMaxDurability;
    if (nMaxDurability <= 0)
    {
        nCurDurability = 1;
        nMaxDurability = 1;
    }
    if (nCurDurability > nMaxDurability || nCurDurability < 0)
        nCurDurability = nMaxDurability;

    dwShopTemplateID = pPlayer->m_ItemList.GetTimeLimitReturnItemShopTemplateID(pItem->m_dwID);
    nShopItemIndex   = pPlayer->m_ItemList.GetTimeLimitReturnItemShopItemIndex(pItem->m_dwID);

    pShopItem = g_pSO3World->m_ShopCenter.GetShopTemplateItem(dwShopTemplateID, nShopItemIndex);
    KG_PROCESS_ERROR(pShopItem);

    pItem     = NULL;
    pItemInfo = NULL;

    bRetCode = pPlayer->m_ItemList.CanDestroyItem(dwBox, dwX);
    if (bRetCode != ircSuccess)
    {
        nResult = ssrcReturnItemSelfCannotDestroy;
        goto Exit0;
    }

    if (pShopItem->dwTabType && pShopItem->dwIndex && pShopItem->nRequireAmount > 0)
    {
        pAddItem = g_pSO3World->GenerateItem(pShopItem->dwTabType, pShopItem->dwIndex);
        KGLOG_PROCESS_ERROR(pAddItem);

        pAddItemInfo = pAddItem->GetItemInfo();
        KGLOG_PROCESS_ERROR(pAddItemInfo);

        // TARGET QUIRK (0x081427d5): this guard jumps straight to Exit0 without freeing
        // pAddItem. The leak is target behaviour and is preserved deliberately.
        KG_PROCESS_ERROR(!(pShopItem->nRequireAmount > pAddItem->GetMaxStackNum()));

        if (pAddItem->IsStackable())
            pAddItem->SetStackNum(pShopItem->nRequireAmount);

        bRetCode = pPlayer->m_ItemList.CanAddItem(pAddItem, false);

        g_pSO3World->m_ItemManager.FreeItem(pAddItem);
        pAddItem     = NULL;
        pAddItemInfo = NULL;

        if (bRetCode != aircSuccess)
        {
            nResult = ssrcReturnItemOtherCannotAdd;
            goto Exit0;
        }
    }

    if (pShopItem->nPrice > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nPrice / (int64_t)nMaxDurability;
        int64_t nRoom = (int64_t)pPlayer->m_ItemList.GetMoneyLimit() - (int64_t)pPlayer->m_ItemList.GetMoney();
        if (nRoom < nGain)
        {
            nResult = ssrcHaveTooMuchMoney;
            goto Exit0;
        }
    }

    if (pShopItem->nAchievementPoint > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nAchievementPoint / (int64_t)nMaxDurability;
        int64_t nRoom = (int64_t)0x7fffffff - (int64_t)pPlayer->m_Achievement.m_nPoint;
        if (nRoom < nGain)
        {
            nResult = ssrcHaveTooMuchAchievementPoint;
            goto Exit0;
        }
    }

    if (pShopItem->nMentorValue > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nMentorValue / (int64_t)nMaxDurability;
        int64_t nRoom = (int64_t)g_pSO3World->m_Settings.m_ConstList.nMaxUsableMentorValue
                      - (int64_t)pPlayer->m_nUsableMentorValue;
        if (nRoom < nGain)
        {
            nResult = ssrcHaveTooMuchMentorValue;
            goto Exit0;
        }
    }

    memset(nCostList, 0, sizeof(nCostList));
    nCostList[0] = pShopItem->nContribution;
    nCostList[1] = pShopItem->nPrestige;
    nCostList[2] = pShopItem->nJustice;
    nCostList[3] = pShopItem->nExamPrint;
    nCostList[4] = pShopItem->nArenaAward;
    nCostList[5] = pShopItem->nActivityAward;

    memset(nCodeList, 0, sizeof(nCodeList));
    nCodeList[0] = ssrcHaveTooMuchContribution;
    nCodeList[1] = ssrcHaveTooMuchPrestige;
    nCodeList[2] = ssrcHaveTooMuchJustice;
    nCodeList[3] = ssrcHaveTooMuchExamPrint;
    nCodeList[4] = ssrcHaveTooMuchArenaAward;
    nCodeList[5] = ssrcHaveTooMuchActivityAward;

    for (i = 0; i <= 5; ++i)
    {
        KCurrency*  pCurrency = NULL;
        int64_t     nGain     = (int64_t)nCostList[i];
        int64_t     nRoom     = 0;

        if (nCostList[i] == 0)
            continue;

        pCurrency = pPlayer->m_CurrencyList.GetCurrency(i);
        KGLOG_PROCESS_ERROR(pCurrency);

        nGain = (int64_t)nCurDurability * nGain / (int64_t)nMaxDurability;
        nRoom = (int64_t)(pCurrency->GetMaxValue() - pCurrency->GetValue());

        if (nRoom < nGain)
        {
            nResult = (SHOP_SYSTEM_RESPOND_CODE)nCodeList[i];
            goto Exit0;
        }
    }

    nResult = ssrcReturnSuccess;
Exit0:
    return nResult;
}

#ifdef _SERVER
BOOL KShop::RepairItem(KPlayer* pPlayer, DWORD dwBox, DWORD dwX, DWORD dwItemID)
{
    BOOL    bResult = false;
    BOOL    bRetCode = false;
    KItem*  pItem = NULL;
    int     nBoxType = 0;
    int     nCost = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    KGLOG_PROCESS_ERROR(m_nShopType == eShopType_NPC);
    KGLOG_PROCESS_ERROR(m_bCanRepair);

    nBoxType = pPlayer->m_ItemList.GetBoxType(dwBox);
    KGLOG_PROCESS_ERROR(nBoxType == ivtEquipment || nBoxType == ivtPackage);

    pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
    KGLOG_PROCESS_ERROR(pItem);
    KGLOG_PROCESS_ERROR(pItem->m_dwID == dwItemID);
    KGLOG_PROCESS_ERROR(pItem->IsRepairable());

    nCost = GetPlayerRepairPrice(pPlayer, dwBox, dwX);
    KGLOG_PROCESS_ERROR(nCost >= 0);

    bRetCode = pPlayer->m_ItemList.Repair(dwBox, dwX);
    KGLOG_PROCESS_ERROR(bRetCode);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KShop::RepairAllItems(KPlayer* pPlayer)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    KGLOG_PROCESS_ERROR(m_nShopType == eShopType_NPC);
    KGLOG_PROCESS_ERROR(m_bCanRepair);

    for (DWORD dwBox = 0; dwBox < ibTotal; ++dwBox)
    {
        int nBoxType = pPlayer->m_ItemList.GetBoxType(dwBox);
        if (nBoxType != ivtEquipment && nBoxType != ivtPackage)
            continue;

        DWORD dwBoxSize = pPlayer->m_ItemList.GetBoxSize(dwBox);
        for (DWORD dwX = 0; dwX < dwBoxSize; ++dwX)
        {
            KItem* pItem = pPlayer->m_ItemList.GetItem(dwBox, dwX);
            if (!pItem || !pItem->IsRepairable())
                continue;

            KGLOG_PROCESS_ERROR(RepairItem(pPlayer, dwBox, dwX, pItem->m_dwID));
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShop::BuyItem(KPlayer* pPlayer, KSHOP_BUY_ITEM_PARAM& rParam)
{
    BOOL                        bResult             = false;
    BOOL                        bRetCode            = false;
    KSHOP_ITEM*                 pShopItem           = NULL;
    KItem*                      pItemBuy            = NULL; 
    BOOL                        bSyncEntireItem     = false;
    KNPC_SHOP_TEMPLATE_ITEM*    pItemTemplate       = NULL;  
    KITEM_LOG_INFO              ItemLogInfo;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(rParam.dwPageIndex < m_dwValidPage);
    KGLOG_PROCESS_ERROR(rParam.dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT);
    
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    
    bRetCode = CanBuyItem(pPlayer, m_pNpc, rParam);
    if (bRetCode != ssrcBuySuccess)
    {
        g_PlayerServer.DoSyncShopItem(
            pPlayer->m_nConnIndex, this, rParam.dwPageIndex, rParam.dwPosIndex, true
        );
        goto Exit0;
    }

    pShopItem = &m_ShopPages[rParam.dwPageIndex].ShopItems[rParam.dwPosIndex];
    KGLOG_PROCESS_ERROR(pShopItem->pItem);
    
    // ============== ִ�й��� ============================>
    
    pItemTemplate = g_pSO3World->m_ShopCenter.GetShopTemplateItem(m_dwTemplateID, pShopItem->nItemTemplateIndex);
    KGLOG_PROCESS_ERROR(pItemTemplate);

    if (pItemTemplate->nLimit != -1)
    {
        assert(rParam.nCount <= pShopItem->nCount);
        pShopItem->nCount -= rParam.nCount;
    }

    pItemBuy = g_pSO3World->m_ItemManager.CloneItem(pShopItem->pItem);
    KGLOG_PROCESS_ERROR(pItemBuy);

    if (pItemBuy->IsStackable())
    {
        bRetCode = pItemBuy->SetStackNum(rParam.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = pPlayer->m_ItemList.AddMoney(-rParam.nCost);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LogClient.LogPlayerMoneyChange(-rParam.nCost, pPlayer, "buy item from npc");

    g_pSO3World->m_StatDataServer.UpdateMoneyStat(pPlayer, -rParam.nCost, "BUY_ITEM");

    if (pItemTemplate->nPrestige > 0)
    {
        bRetCode = pPlayer->AddPrestige(-pItemTemplate->nPrestige * rParam.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pPlayer, -pItemTemplate->nPrestige * rParam.nCount, "SHOP");
    }

    if (pItemTemplate->nContribution > 0)
    {
        int nOldContribution = 0;
        int nNewContribution = 0;

        nOldContribution = pPlayer->m_nContribution;
        bRetCode = pPlayer->AddContribution(-pItemTemplate->nContribution * rParam.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
        nNewContribution = pPlayer->m_nContribution;

        if (nNewContribution != nOldContribution)
            g_pSO3World->m_StatDataServer.UpdateContributeStat(pPlayer, nNewContribution - nOldContribution, "BUY_ITEM");
    }

    // v2.5: deduct yuanbao + the 4 new currencies (CanBuyItem already gated affordability server-side).
    if (pItemTemplate->nCoin > 0)
    {
        bRetCode = pPlayer->AddCoin(-pItemTemplate->nCoin * rParam.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    {
        int nCurIdx = 0;
        for (nCurIdx = 0; nCurIdx < 4; nCurIdx++)
        {
            int        nType    = 2 + nCurIdx;   // cbtJustice=2 .. cbtActivityAward=5
            int        nCurCost = 0;
            KCurrency* pCur     = NULL;
            switch (nCurIdx)
            {
            case 0: nCurCost = pItemTemplate->nJustice;       break;
            case 1: nCurCost = pItemTemplate->nExamPrint;     break;
            case 2: nCurCost = pItemTemplate->nArenaAward;    break;
            default:nCurCost = pItemTemplate->nActivityAward; break;
            }
            if (nCurCost <= 0)
                continue;
            pCur = pPlayer->m_CurrencyList.GetCurrency(nType);
            KGLOG_PROCESS_ERROR(pCur);
            bRetCode = pCur->AddCurrency(-nCurCost * rParam.nCount);   // spend (negative, clamps >=0)
            KGLOG_PROCESS_ERROR(bRetCode);
            // v2.5: DoSyncCurrency to client here (deferred packet).
        }
    }

    if (pItemTemplate->nAchievementPoint > 0)
    {
        bRetCode = pPlayer->m_Achievement.AddPoint(-pItemTemplate->nAchievementPoint * rParam.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    if (pItemTemplate->dwTabType > 0 && pItemTemplate->dwIndex > 0 && pItemTemplate->nRequireAmount > 0)
    {
        bRetCode = pPlayer->m_ItemList.CostItemInEquipAndPackage(
            pItemTemplate->dwTabType, pItemTemplate->dwIndex, pItemTemplate->nRequireAmount * rParam.nCount
        );
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    g_LogClient.GetItemLogInfo(pItemBuy, &ItemLogInfo);
    bRetCode = pPlayer->m_ItemList.AddItem(pItemBuy);
    KGLOG_PROCESS_ERROR(bRetCode == aircSuccess);
    g_LogClient.LogPlayerItemChange(pPlayer, ITEM_OPERATION_TYPE_IN, &ItemLogInfo, "buy from npc");
    pItemBuy = NULL;
    
    // <============== ִ�й��� ============================

    // ���һ���̵겻ΪNPC�̵�򱻹������Ʒ�������ģ�����ҹ���󣬸ø�����Ʒ�������϶��Ǹı��˵�
    if ((m_nShopType != eShopType_NPC) || (pItemTemplate->nLimit != -1))
    {
        if ((m_nShopType == eShopType_PLAYER) && (pShopItem->nCount == 0))
        {
            // ���һ�̵�Ϊ����̵꣬ĳ��Ʒ�������ĸ���Ϊ0ʱ�����轫��Ʒ���̵���ȥ��
            bRetCode = DeleteItem(rParam.dwPageIndex, rParam.dwPosIndex);
            KGLOG_PROCESS_ERROR(bRetCode);

            bSyncEntireItem = true;
        }

        g_PlayerServer.DoSyncShopItem(
            pPlayer->m_nConnIndex, this, rParam.dwPageIndex, rParam.dwPosIndex, bSyncEntireItem
        );
    }

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pItemBuy)
        {
            g_pSO3World->m_ItemManager.FreeItem(pItemBuy);
            pItemBuy = NULL;
        }
    }
    return bResult;
}

BOOL KShop::BuyCoinShopItem(KPlayer* pPlayer, KSHOP_BUY_ITEM_PARAM& rParam)
{
    BOOL                    bResult = false;
    // target SHOP_SYSTEM_RESPOND_CODE has no ssrcInvalidParam (0 DWARF hits); index 0 of the
    // target enum is ssrcInvalid, which is what the candidate enum also defines (SO3Result.h).
    SHOP_SYSTEM_RESPOND_CODE eResult = ssrcInvalid;
    KSHOP_ITEM*             pShopItem = NULL;
    KNPC_SHOP_TEMPLATE_ITEM* pTemplateItem = NULL;
    int                     nRandomSeed = 0;
    int                     nCoinCost = 0;

    KGLOG_PROCESS_ERROR(m_bCoinShop);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(rParam.dwPageIndex < m_dwValidPage);
    KGLOG_PROCESS_ERROR(rParam.dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT);
    KGLOG_PROCESS_ERROR(rParam.nCount < 17);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);

    eResult = CanBuyItem(pPlayer, m_pNpc, rParam);
    if (eResult != ssrcBuySuccess)
    {
        g_PlayerServer.DoSyncShopItem(
            pPlayer->m_nConnIndex, this, rParam.dwPageIndex, rParam.dwPosIndex, true
        );
        goto Exit0;
    }

    pShopItem = &m_ShopPages[rParam.dwPageIndex].ShopItems[rParam.dwPosIndex];
    KGLOG_PROCESS_ERROR(pShopItem->pItem);

    pTemplateItem = g_pSO3World->m_ShopCenter.GetShopTemplateItem(
        m_dwTemplateID, pShopItem->nItemTemplateIndex
    );
    KGLOG_PROCESS_ERROR(pTemplateItem);

    if (!pShopItem->pItem->IsStackable())
        nRandomSeed = (int)pShopItem->pItem->m_GenParam.dwRandSeed;

    nCoinCost = pTemplateItem->nCoin * rParam.nCount;
    KGLOG_PROCESS_ERROR(nCoinCost >= 0);
    KGLOG_PROCESS_ERROR(pPlayer->m_nCoin >= nCoinCost);
    KGLOG_PROCESS_ERROR(pPlayer->AddCoin(-nCoinCost));

    KGLOG_PROCESS_ERROR(g_RelayClient.DoCoinShopBuyItemRequest(
        pPlayer,
        pShopItem->pItem->m_GenParam.dwTabType,
        pShopItem->pItem->m_GenParam.dwIndex,
        nRandomSeed,
        rParam.nCount,
        pTemplateItem->nCoin,
        0,
        0
    ));

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShop::SellItem(KPlayer* pPlayer, KSHOP_SELL_ITEM_PARAM& rParam)
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    KItem*      pItem       = NULL;
    int         nPrice      = 0;
    int         nCount      = 0;
    int         nBoxType    = 0;
    int         nCost       = 0;
    int         nMoney      = 0;
    int         nMaxMoney   = 0;
    DWORD       dwX         = 0;
    TItemPos    SrcPos;
    TItemPos    DestPos;

    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);

    nBoxType = pPlayer->m_ItemList.GetBoxType(rParam.dwBox);
    KGLOG_PROCESS_ERROR(nBoxType == ivtEquipment || nBoxType == ivtPackage);

    // ȷ�����ȷʵ�������Ʒ(�˶���ƷID)
    pItem = pPlayer->m_ItemList.GetItem(rParam.dwBox, rParam.dwX);
    KGLOG_PROCESS_ERROR(pItem);
    KGLOG_PROCESS_ERROR(pItem->m_dwID == rParam.dwItemID);

    // ȷ����Ʒ�Ƿ����: ��Ʒ"�ɽ���"����, 
    nRetCode = pItem->IsCanTrade();
    KGLOG_PROCESS_ERROR(nRetCode);
    
    nCount = pItem->GetStackNum();
    KGLOG_PROCESS_ERROR(nCount > 0);

    nPrice = GetPlayerSellPrice(pPlayer, rParam.dwBox, rParam.dwX);
    KGLOG_PROCESS_ERROR(nPrice >= 0);

    nCost = nPrice * nCount;
    KG_ASSERT_EXIT(nCost >= 0);      // ���
    // �˶Լ�ֵ�Ƿ����ҿ�����һ��
    KGLOG_PROCESS_ERROR(nCost == rParam.nCost);

    // ȷ��������Ͻ�Ǯ���ᳬ������
    nMoney = pPlayer->m_ItemList.GetMoney();
    nMaxMoney = pPlayer->m_ItemList.GetMoneyLimit();
    KGLOG_PROCESS_ERROR(nMoney <= nMaxMoney - nCost);    // ������

    // ת����Ʒ�ͽ�Ǯ
    switch (m_nShopType)
    {
    case eShopType_NPC:
        // ���ر�
        nRetCode = pPlayer->m_ItemList.FindFreeSoldList(dwX);
        KGLOG_PROCESS_ERROR(nRetCode);

        SrcPos.dwBox = rParam.dwBox;
        SrcPos.dwX   = rParam.dwX;
        DestPos.dwBox = ibSoldList;
        DestPos.dwX   = dwX;
        
        pPlayer->m_ItemList.m_bSoldListOpened = true;
        nRetCode = pPlayer->m_ItemList.ExchangeItem(SrcPos, DestPos);
        KGLOG_PROCESS_ERROR(nRetCode == ircSuccess);
        pPlayer->m_ItemList.m_bSoldListOpened = false;

        nRetCode = pPlayer->m_ItemList.AddMoney(nCost);
        KG_ASSERT_EXIT(nRetCode);

        g_LogClient.LogPlayerMoneyChange(nCost, pPlayer, "sell item to npc");

        g_pSO3World->m_StatDataServer.UpdateMoneyStat(pPlayer, nCost, "SELL_ITEM");

        break;
    default:
        KG_ASSERT_EXIT(!"δ������̵�����");
    }

    g_LogClient.LogPlayerItemChange(pPlayer, ITEM_OPERATION_TYPE_OUT, pItem, "sell to npc");

    bResult = true;
Exit0:
    return bResult;
}

// Ported 1:1 from target KShop::ReturnItem @0x08142d92 .. 0x08143a8x.
BOOL KShop::ReturnItem(KPlayer* pPlayer, KSHOP_SELL_ITEM_PARAM& rParam)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    ITEM_RESULT_CODE            nDestroyRet = ircFailed;
    SHOP_SYSTEM_RESPOND_CODE    nCode       = ssrcReturnFailed;
    KItem*                      pItem       = NULL;
    KItemInfo*                  pItemInfo   = NULL;
    int                         nBoxType    = 0;
    DWORD                       dwItemID    = 0;
    DWORD                       dwShopTemplateID = 0;
    int                         nShopItemIndex   = 0;
    KNPC_SHOP_TEMPLATE_ITEM*    pShopItem   = NULL;
    int                         nMaxDurability = 0;
    int                         nCurDurability = 0;
    KItem*                      pAddItem    = NULL;
    KItemInfo*                  pAddItemInfo = NULL;
    const char*                 pszStatSub  = NULL;
    TItemPos                    DestroyPos;
    int                         nCostList[6];
    int                         i           = 0;

    KG_PROCESS_ERROR(g_pSO3World->m_bReturnItemFlag);
    KGLOG_PROCESS_ERROR(pPlayer);
    KG_PROCESS_ERROR(!pPlayer->m_bFightState);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);

    nBoxType = pPlayer->m_ItemList.GetBoxType(rParam.dwBox);
    KGLOG_PROCESS_ERROR(nBoxType == ivtEquipment || nBoxType == ivtPackage);

    pItem = pPlayer->m_ItemList.GetItem(rParam.dwBox, rParam.dwX);
    KG_PROCESS_ERROR(pItem);
    KG_PROCESS_ERROR(pItem->m_dwID == rParam.dwItemID);

    dwItemID         = pItem->m_dwID;
    dwShopTemplateID = pPlayer->m_ItemList.GetTimeLimitReturnItemShopTemplateID(dwItemID);
    nShopItemIndex   = pPlayer->m_ItemList.GetTimeLimitReturnItemShopItemIndex(dwItemID);
    pShopItem        = g_pSO3World->m_ShopCenter.GetShopTemplateItem(dwShopTemplateID, nShopItemIndex);

    // target 0x08143021: expired / no longer returnable -> drop the metadata and bail
    // silently with bResult == false.
    if (pShopItem == NULL || !pShopItem->bCanReturn)
    {
        pPlayer->m_ItemList.DelTimeLimitReturnItemInfo(dwItemID);
        goto Exit0;
    }

    nCode = CanReturnItem(pPlayer, rParam.dwBox, rParam.dwX);
    KG_PROCESS_ERROR(nCode == ssrcReturnSuccess);

    pItemInfo = pItem->GetItemInfo();
    KG_PROCESS_ERROR(pItemInfo);

    if (pItemInfo->nSub == 6 || pItemInfo->nSub == 7 || pItemInfo->nSub == 8)
        pszStatSub = "EQUIPMENT";
    else if (pItemInfo->nDetail == 10)
        pszStatSub = "DIAMOND";
    else if (pItemInfo->nSub == 5)
        pszStatSub = "OTHER";

    nCurDurability = pItem->GetCurrentDurability();
    nMaxDurability = pItemInfo->nMaxDurability;
    if (nMaxDurability <= 0)
    {
        nCurDurability = 1;
        nMaxDurability = 1;
    }
    if (nCurDurability > nMaxDurability || nCurDurability < 0)
        nCurDurability = nMaxDurability;

    if (pItemInfo->nQuality >= g_pSO3World->m_Settings.m_ConstList.nSellToNpcItemQualityToLog)
    {
        g_LogClient.LogPlayerItemChange(pPlayer, ITEM_OPERATION_TYPE_OUT, pItem, "return to npc");
    }

    DestroyPos.dwBox = rParam.dwBox;
    DestroyPos.dwX   = rParam.dwX;
    nDestroyRet = pPlayer->m_ItemList.DestroyItem(DestroyPos);
    KGLOG_PROCESS_ERROR(nDestroyRet == ircSuccess);

    pItem     = NULL;
    pItemInfo = NULL;

    if (pShopItem->dwTabType && pShopItem->dwIndex && pShopItem->nRequireAmount > 0)
    {
        pAddItem = g_pSO3World->GenerateItem(pShopItem->dwTabType, pShopItem->dwIndex);
        KGLOG_PROCESS_ERROR(pAddItem);

        pAddItemInfo = pAddItem->GetItemInfo();
        KGLOG_PROCESS_ERROR(pAddItemInfo);
        KGLOG_PROCESS_ERROR(pShopItem->nRequireAmount <= pAddItem->GetMaxStackNum());

        if (pAddItem->IsStackable())
            pAddItem->SetStackNum(pShopItem->nRequireAmount);

        if (pAddItemInfo->nQuality >= g_pSO3World->m_Settings.m_ConstList.nSellToNpcItemQualityToLog)
        {
            g_LogClient.LogPlayerItemChange(pPlayer, ITEM_OPERATION_TYPE_IN, pAddItem, "return from npc");
        }

        bRetCode = pPlayer->m_ItemList.AddItem(pAddItem);
        KGLOG_PROCESS_ERROR(bRetCode == aircSuccess);

        pAddItem     = NULL;
        pAddItemInfo = NULL;
    }

    if (pShopItem->nPrice > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nPrice / (int64_t)nMaxDurability;
        if (nGain > 0)
        {
            pPlayer->m_ItemList.AddMoney((int)nGain);
            g_LogClient.LogPlayerMoneyChange((int)nGain, pPlayer, "return item from npc");
            g_pSO3World->m_StatDataServer.UpdateMoneyStat((int)nGain, "RETURN_ITEM", pszStatSub);
        }
    }

    if (pShopItem->nMentorValue > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nMentorValue / (int64_t)nMaxDurability;
        if (nGain > 0)
        {
            pPlayer->AddUsableMentorValue((int)nGain);
            g_LogClient.LogUsableMentorValueChange((int)nGain, pPlayer, "return item from npc.");
        }
    }

    if (pShopItem->nAchievementPoint > 0)
    {
        int64_t nGain = (int64_t)nCurDurability * (int64_t)pShopItem->nAchievementPoint / (int64_t)nMaxDurability;
        if (nGain > 0)
            pPlayer->m_Achievement.AddPoint((int)nGain);
    }

    memset(nCostList, 0, sizeof(nCostList));
    nCostList[0] = pShopItem->nContribution;
    nCostList[1] = pShopItem->nPrestige;
    nCostList[2] = pShopItem->nJustice;
    nCostList[3] = pShopItem->nExamPrint;
    nCostList[4] = pShopItem->nArenaAward;
    nCostList[5] = pShopItem->nActivityAward;

    for (i = 0; i <= 5; ++i)
    {
        KCurrency*  pCurrency = NULL;
        int64_t     nGain     = (int64_t)nCostList[i] * (int64_t)nCurDurability / (int64_t)nMaxDurability;

        if (nGain <= 0)
            continue;

        pCurrency = pPlayer->m_CurrencyList.GetCurrency(i);
        if (pCurrency == NULL)
            continue;

        pCurrency->AddRemainSpace((int)nGain);
        pCurrency->AddCurrency((int)nGain);

        // PORT-UNKNOWN_REQUIRED[STATE] target=KShop::ReturnItem@0x08143715-0x08143843.
        // The target builds a 6-slot array of pointer-to-member stat updaters with SEVEN
        // writes: slot0 is written with UpdateContributeStat and then overwritten at
        // 0x0814381d with UpdateActivityAwardStat, leaving
        //   [0]=ActivityAward [1]=Prestige [2]=Justice [3]=ExamPrint [4]=ArenaAward
        //   [5]=Contribute
        // which is index-swapped at 0 and 5 relative to nCostList / nCodeList above
        // (0=Contribution .. 5=ActivityAward). Reproduced verbatim, so a CONTRIBUTION
        // refund is recorded under ACTIVITYAWARD and an ACTIVITYAWARD refund under
        // CONTRIBUTE. Stat keys only -- currency granted to the player is unaffected.
        // Next evidence needed: the 2.5.2 source or a stat dump proving whether the
        // double-write is an upstream bug or intentional.
        switch (i)
        {
        case 0: g_pSO3World->m_StatDataServer.UpdateActivityAwardStat(pPlayer, (int)nGain, "RETURN_ITEM"); break;
        case 1: g_pSO3World->m_StatDataServer.UpdatePrestigeStat(pPlayer, (int)nGain, "RETURN_ITEM");      break;
        case 2: g_pSO3World->m_StatDataServer.UpdateJusticeStat(pPlayer, (int)nGain, "RETURN_ITEM");       break;
        case 3: g_pSO3World->m_StatDataServer.UpdateExamPrintStat(pPlayer, (int)nGain, "RETURN_ITEM");     break;
        case 4: g_pSO3World->m_StatDataServer.UpdateArenaAwardStat(pPlayer, (int)nGain, "RETURN_ITEM");    break;
        case 5: g_pSO3World->m_StatDataServer.UpdateContributeStat(pPlayer, (int)nGain, "RETURN_ITEM");    break;
        }
    }

    bResult = true;
Exit0:
    // target 0x081439e2
    if (pPlayer)
    {
        if (bResult)
        {
            pPlayer->m_ItemList.DelTimeLimitReturnItemInfo(dwItemID);
        }
        else
        {
            g_PlayerServer.DoMessageNotify(
                pPlayer->m_nConnIndex, ectShopEventNotifyCode, nCode, NULL, 0
            );
        }
    }

    if (!bResult && pAddItem)
    {
        g_pSO3World->m_ItemManager.FreeItem(pAddItem);
        pAddItem = NULL;
    }

    return bResult;
}

BOOL KShop::RepairItem(KPlayer* pPlayer, KSHOP_REPAIR_ITEM_PARAM& rParam)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KItem*  pItem       = NULL;
    int     nBoxType    = 0;
    int     nCost       = 0;
    int     nMoney      = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);

    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);
    KG_PROCESS_ERROR(m_bCanRepair);
    
    nBoxType = pPlayer->m_ItemList.GetBoxType(rParam.dwBox);
    KGLOG_PROCESS_ERROR(nBoxType == ivtEquipment || nBoxType == ivtPackage);

    pItem = pPlayer->m_ItemList.GetItem(rParam.dwBox, rParam.dwX);
    KGLOG_PROCESS_ERROR(pItem);
    KGLOG_PROCESS_ERROR(pItem->m_dwID == rParam.dwItemID);

    KGLOG_PROCESS_ERROR(pItem->IsRepairable());

    nCost = GetPlayerRepairPrice(pPlayer, rParam.dwBox, rParam.dwX);
    KGLOG_PROCESS_ERROR(nCost >= 0);

    if (nCost != rParam.nCost)
    {   
        g_PlayerServer.DoSyncItemData(pPlayer->m_nConnIndex, pPlayer->m_dwID, pItem, rParam.dwBox, rParam.dwX);
        goto Exit0;
    }

    nMoney = pPlayer->m_ItemList.GetMoney();
    KGLOG_PROCESS_ERROR(nMoney >= nCost);

    bRetCode = pPlayer->m_ItemList.Repair(rParam.dwBox, rParam.dwX);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->m_ItemList.AddMoney(-nCost);
    assert(bRetCode);

    g_LogClient.LogPlayerMoneyChange(-nCost, pPlayer, "repair item");

    g_pSO3World->m_StatDataServer.UpdateMoneyStat(pPlayer, -nCost, "REPAIR_ITEM");
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL KShop::RepairAllItems(KPlayer* pPlayer, int nCost)
{
    BOOL bResult        = false;
    BOOL bRetCode       = false;
    int  nNeedCost      = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KGLOG_PROCESS_ERROR(m_pNpc->m_dwID == m_dwNpcID);

    // ȷ���̵��Ƿ������������Ʒ(����̵겻������)
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);
    KG_PROCESS_ERROR(m_bCanRepair);
    
    KG_PROCESS_SUCCESS(nCost == 0);

    // ��������۸�
    for (DWORD dwBoxIndex = 0; dwBoxIndex < ibTotal; ++dwBoxIndex)
    {
        int nBoxType = pPlayer->m_ItemList.GetBoxType(dwBoxIndex);
        if (nBoxType == ivtEquipment || nBoxType == ivtPackage)
        {
            DWORD dwBoxSize = pPlayer->m_ItemList.GetBoxSize(dwBoxIndex);

            for (DWORD dwX = 0; dwX < dwBoxSize; dwX++)
            {
                KItem* pItem = pPlayer->m_ItemList.GetItem(dwBoxIndex, dwX);
                if (!pItem)
                    continue;
                
                BOOL bIsRepairable = pItem->IsRepairable();
                if (!bIsRepairable)
                    continue;

                int nPrice = GetPlayerRepairPrice(pPlayer, dwBoxIndex, dwX);
                KGLOG_PROCESS_ERROR(nPrice >= 0);
                
                nNeedCost += nPrice;
                KG_ASSERT_EXIT(nNeedCost >= 0); // ������
            }
        }
    }

    KG_PROCESS_ERROR(nNeedCost == nCost);

    for (DWORD dwBoxIndex = 0; dwBoxIndex < ibTotal; ++dwBoxIndex)
    {
        int nBoxType = pPlayer->m_ItemList.GetBoxType(dwBoxIndex);
        if (nBoxType == ivtEquipment || nBoxType == ivtPackage)
        {
            DWORD dwBoxSize = pPlayer->m_ItemList.GetBoxSize(dwBoxIndex);

            for (DWORD dwX = 0; dwX < dwBoxSize; dwX++)
            {
                KItem* pItem = pPlayer->m_ItemList.GetItem(dwBoxIndex, dwX);
                if (!pItem)
                    continue;
            
                BOOL bIsRepairable = pItem->IsRepairable();
                if (!bIsRepairable)
                    continue;
                
                KSHOP_REPAIR_ITEM_PARAM Param;
                Param.dwBox = dwBoxIndex;
                Param.dwX = dwX;
                Param.dwItemID = pItem->m_dwID;
                Param.nCost = GetPlayerRepairPrice(pPlayer, dwBoxIndex, dwX);

                bRetCode = RepairItem(pPlayer, Param);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
        }
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KShop::BuySoldListItem(KPlayer* pPlayer, KSHOP_BUY_SOLD_LIST_ITEM_PARAM& rParam)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KItem*      pItem       = NULL;
    int         nPrice      = 0;
    int         nCost       = 0;
    int         nMoney      = 0;
    DWORD       dwBoxIndex  = 0;
    DWORD       dwX         = 0;
    TItemPos    SrcPos;
    TItemPos    DestPos;
    int         nNext       = 0;
    KItemInfo*  pItemInfo   = NULL;
    int         nItemAmount = 0;
    KITEM_LOG_INFO  ItemLogInfo;

    KG_ASSERT_EXIT(pPlayer);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);

    // �����Ʒ�Ƿ��иı�
    pItem = pPlayer->m_ItemList.GetItem(ibSoldList, rParam.dwX);
    KGLOG_PROCESS_ERROR(pItem);
    KGLOG_PROCESS_ERROR(pItem->m_dwID == rParam.dwItemID);

    // �������Ƿ����㹻��Ǯ
    nPrice = GetPlayerSellPrice(pPlayer, ibSoldList, rParam.dwX);
    KGLOG_PROCESS_ERROR(nPrice >= 0);

    nCost = nPrice * pItem->GetStackNum();
    KG_ASSERT_EXIT(nCost >= 0);
    
    nMoney = pPlayer->m_ItemList.GetMoney();
    KGLOG_PROCESS_ERROR(nCost <= nMoney);

    bRetCode = pPlayer->m_ItemList.FindFreeRoomInPackage(dwBoxIndex, dwX);
    KGLOG_PROCESS_ERROR(bRetCode);

    nItemAmount = pPlayer->m_ItemList.GetItemTotalAmount(
        pItem->m_GenParam.dwTabType, pItem->m_GenParam.dwIndex
    );

    pItemInfo = pItem->GetItemInfo();
    KGLOG_PROCESS_ERROR(pItemInfo);
    g_LogClient.GetItemLogInfo(pItem, &ItemLogInfo);
    
    KG_PROCESS_ERROR(pItemInfo->nMaxExistAmount == 0 || (nItemAmount + pItem->GetStackNum()) <= pItemInfo->nMaxExistAmount);

    SrcPos.dwBox = ibSoldList;
    SrcPos.dwX = rParam.dwX;
    DestPos.dwBox = dwBoxIndex;
    DestPos.dwX = dwX;
    
    pPlayer->m_ItemList.m_bSoldListOpened = true;

    bRetCode = pPlayer->m_ItemList.ExchangeItem(SrcPos, DestPos);
    KGLOG_PROCESS_ERROR(bRetCode == ircSuccess);

    bRetCode = pPlayer->m_ItemList.AddMoney(-nCost);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LogClient.LogPlayerMoneyChange(-nCost, pPlayer, "buy soldlist item");

    g_pSO3World->m_StatDataServer.UpdateMoneyStat(pPlayer, -nCost, "BUY_ITEM");

    // ���������ص���Ʒ�ǻع��б��м��ĳ��������ع������Ա�֤KItemList::FindFreeSoldList()����ȷ�ԡ�
    nNext = (rParam.dwX + 1) >= MAX_SOLDLIST_PACKAGE_SIZE ? 0 : (rParam.dwX + 1);
    pItem = pPlayer->m_ItemList.GetItem(ibSoldList, nNext);
    if (pItem)
    {
        pPlayer->m_ItemList.TidyUpSoldList();
    }
    
    pPlayer->m_ItemList.m_bSoldListOpened = false;
    
    g_LogClient.LogPlayerItemChange(pPlayer, ITEM_OPERATION_TYPE_IN, &ItemLogInfo, "buy from sold list");

    bResult = true;
Exit0:
    return bResult;
}

// Ported 1:1 from target KShop::BuyTimeLimitSoldListItem @0x08140f10.
// Target box constant is 0x11 == ibTimeLimitSoldList (NOT ibSoldList == 12).
BOOL KShop::BuyTimeLimitSoldListItem(KPlayer* pPlayer, KSHOP_BUY_TIME_LIMIT_SOLD_LIST_ITEM_PARAM& rParam)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KItem*          pItem       = NULL;
    KItemInfo*      pItemInfo   = NULL;
    int             nPrice      = 0;
    int             nCost       = 0;
    int             nMoney      = 0;
    DWORD           dwBoxIndex  = 0;
    DWORD           dwX         = 0;
    const char*     pszStatSub  = NULL;
    int             nItemAmount = 0;
    KITEM_LOG_INFO  ItemLogInfo;
    TItemPos        SrcPos;
    TItemPos        DestPos;

    KG_ASSERT_EXIT(pPlayer);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bFightState);
    KGLOG_PROCESS_ERROR(m_pNpc);
    KG_PROCESS_ERROR(m_nShopType == eShopType_NPC);

    pItem = pPlayer->m_ItemList.GetItem(ibTimeLimitSoldList, rParam.dwX);
    KGLOG_PROCESS_ERROR(pItem);
    KGLOG_PROCESS_ERROR(pItem->m_dwID == rParam.dwItemID);

    pItemInfo = pItem->GetItemInfo();
    KGLOG_PROCESS_ERROR(pItemInfo);

    if (pItemInfo->nSub == 6 || pItemInfo->nSub == 7 || pItemInfo->nSub == 8)
        pszStatSub = "EQUIPMENT";
    else if (pItemInfo->nDetail == 10)
        pszStatSub = "DIAMOND";
    else if (pItemInfo->nSub == 5)
        pszStatSub = "OTHER";

    nPrice = GetPlayerSellPrice(pPlayer, ibTimeLimitSoldList, rParam.dwX);
    KGLOG_PROCESS_ERROR(nPrice >= 0);

    nCost = pItem->GetStackNum() * nPrice;
    KG_ASSERT_EXIT(nCost >= 0);

    nMoney = pPlayer->m_ItemList.GetMoney();
    KGLOG_PROCESS_ERROR(nCost <= nMoney);

    bRetCode = pPlayer->m_ItemList.FindFreeRoomInPackage(dwBoxIndex, dwX);
    KGLOG_PROCESS_ERROR(bRetCode);

    nItemAmount = pPlayer->m_ItemList.GetItemTotalAmount(
        pItem->m_GenParam.dwTabType, pItem->m_GenParam.dwIndex
    );

    g_LogClient.GetItemLogInfo(pItem, &ItemLogInfo);

    KG_PROCESS_ERROR(
        !(pItemInfo->nMaxExistAmount != 0 &&
          (nItemAmount + pItem->GetStackNum()) > pItemInfo->nMaxExistAmount)
    );

    if (pItemInfo->nDetail == 10)
    {
        g_pSO3World->m_StatDataServer.UpdateDiamondStat(
            pItem->GetStackNum(), pItemInfo->nDetail, "BUY"
        );
    }

    SrcPos.dwBox  = ibTimeLimitSoldList;
    SrcPos.dwX    = rParam.dwX;
    DestPos.dwBox = dwBoxIndex;
    DestPos.dwX   = dwX;

    pPlayer->m_ItemList.m_bSoldListOpened = true;

    bRetCode = pPlayer->m_ItemList.ExchangeItem(SrcPos, DestPos);
    // TARGET QUIRK (0x081413ea): on this failure path m_bSoldListOpened is left set to
    // true -- the target only clears it on the success path. Preserved deliberately.
    KGLOG_PROCESS_ERROR(bRetCode == ircSuccess);

    pPlayer->m_ItemList.DelTimeLimitSoldListInfo(pItem->m_dwID);

    bRetCode = pPlayer->m_ItemList.AddMoney(-nCost);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_ItemList.TidyUpTimeLimitSoldList();

    g_LogClient.LogPlayerMoneyChange(-nCost, pPlayer, "buy soldlist item");

    g_pSO3World->m_StatDataServer.UpdateMoneyStat(-nCost, "BUY_ITEM", pszStatSub);

    pPlayer->m_ItemList.m_bSoldListOpened = false;

    g_LogClient.LogPlayerItemChange(
        pPlayer, ITEM_OPERATION_TYPE_IN, &ItemLogInfo, "buy from sold list"
    );

    bResult = true;
Exit0:
    return bResult;
}

void KShop::Refresh()
{
    KNPC_SHOP_TEMPLATE_ITEM* pItemTemplate = NULL;

    for (DWORD dwPageIndex = 0; dwPageIndex < m_dwValidPage; ++dwPageIndex)
    {
        for (DWORD dwPosIndex = 0; dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT; ++dwPosIndex)
        {
            KSHOP_ITEM* pShopItem = &(m_ShopPages[dwPageIndex].ShopItems[dwPosIndex]);
            
            if (!pShopItem)
                continue;

            if (!pShopItem->pItem)
                continue;
            
            pItemTemplate = g_pSO3World->m_ShopCenter.GetShopTemplateItem(m_dwTemplateID, pShopItem->nItemTemplateIndex);
            KGLOG_PROCESS_ERROR(pItemTemplate);

            if (pShopItem->nCount == pItemTemplate->nLimit)
                continue;
            
            pShopItem->nCount = pItemTemplate->nLimit;
        }
    }

Exit0:
    return;
}
#endif

BOOL KShop::DeleteItem(DWORD dwPageIndex, DWORD dwPosIndex)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KItem*      pItem       = NULL;

    assert(dwPageIndex < MAX_SHOP_PAGES);
    assert(dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT);
    
    pItem = m_ShopPages[dwPageIndex].ShopItems[dwPosIndex].pItem;
    KG_PROCESS_SUCCESS(!pItem);
    
    bRetCode = g_pSO3World->m_ItemManager.FreeItem(pItem);
    KGLOG_PROCESS_ERROR(bRetCode);
    m_ShopPages[dwPageIndex].ShopItems[dwPosIndex].pItem = NULL;
  
    if (m_dwValidPage == (dwPageIndex + 1))
    {
        BOOL bHadItemFlag = false;
        for (int nPosIndex = 0; nPosIndex < MAX_SHOP_PAGE_ITEM_COUNT; ++nPosIndex)
        {
            if (m_ShopPages[dwPageIndex].ShopItems[nPosIndex].pItem)
            {
                bHadItemFlag = true;
                break;
            }
        }
        
        if ((!bHadItemFlag) && m_dwValidPage > 0)
        {
            m_dwValidPage = m_dwValidPage - 1;
        }
    }

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(
            KGLOG_ERR, 
            "[Shop] Try to delete a item from shop %u failed; PageIndex = %u,  PosIndex = %u", 
            m_dwShopID, dwPageIndex, dwPosIndex
        );
    }
    return bResult;
}

KSHOP_ITEM* KShop::GetShopItem(DWORD dwTabType, DWORD dwIndex, DWORD dwRandSeed)
{
    KSHOP_ITEM* pResult     = NULL;
    KItem*      pItem       = NULL;
    KSHOP_ITEM* pShopItem   = NULL;

    for (DWORD dwPageIndex = 0; dwPageIndex < m_dwValidPage; ++dwPageIndex)
    {
        for (DWORD dwPosIndex = 0; dwPosIndex < MAX_SHOP_PAGE_ITEM_COUNT; ++dwPosIndex)
        {
            pShopItem = &m_ShopPages[dwPageIndex].ShopItems[dwPosIndex];
            
            pItem = pShopItem->pItem;
            if (pItem == NULL)
            {
                continue;
            }
            
            if (
                pItem->m_GenParam.dwTabType != dwTabType    || 
                pItem->m_GenParam.dwIndex != dwIndex        || 
                pItem->m_GenParam.dwRandSeed != dwRandSeed
            )
            {
                continue;
            }

            pResult = pShopItem;

            goto Exit0;
        }
    }

Exit0:
    return pResult;
}

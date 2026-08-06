#ifndef _INCLUDE_KSHOPCENTER_H_
#define _INCLUDE_KSHOPCENTER_H_

#include <stdint.h>
#include <map>
#include "GS_Client_Protocol.h"
#include "Engine/KMutex.h"
class KShop;
class KNpc;
class KPlayer;
class ITabFile;

struct KNPC_SHOP_TEMPLATE_ITEM
{
    int   nType;
    int   nIndex;
    DWORD dwRandomSeed;
    int   nLimit;
    int   nDurability;
    int   nReputeLevel;
    int   nPrice;        // ��Ҫ��Ǯ
    int   nCoin;         // v2.5: yuanbao (元宝) price
    int   nContribution; // ��Ҫ�Ĺ���ֵ
    int   nPrestige;     // ��Ҫ������
    int   nJustice;      // v2.5: cbtJustice price
    int   nExamPrint;    // v2.5: cbtExamPrint price
    int   nArenaAward;   // v2.5: cbtArenaAward price
    int   nActivityAward;// v2.5: cbtActivityAward price
    int   nRequireAchievementRecord; // ��Ҫ�ﵽ����Ϸ��ɶ�
    int   nAchievementPoint;         // ��Ҫ�ĳɾ͵���
    int   nTongReputation;
    DWORD dwTabType;        // ��Ҫ����   
    DWORD dwIndex;
    int   nRequireAmount;
    int   nMentorValue;
    int   nRequireTitle;
    int   nRequireCorpsValue;
    DWORD dwMaskCorpsNeedToCheck;
    BOOL  bCanReturn;
};

class KShopCenter
{
public:
    int  Init();
    void UnInit();

    KShop* GetShopInfo(DWORD dwShopID);
    
#ifdef _SERVER
public:
    void Activate();
    KShop* CreateShop(DWORD dwTemplateID);
    void DestroyShop(DWORD dwShopID);
 
private:
    void    DoRefresh();
    BOOL    LoadNpcShopTemplates();
    BOOL    LoadLine(ITabFile* pTabFile, int nLine);
    
    DWORD   m_dwShopID;
    int m_nNextRefreshTime;
#endif

    typedef std::vector<KNPC_SHOP_TEMPLATE_ITEM, KMemory::KAllocator<KNPC_SHOP_TEMPLATE_ITEM> > KNPC_SHOP_TEMPLATE_ITEMS;
    struct KNPC_SHOP_TEMPLATE 
    {
        char                        szShopName[_NAME_LEN];
        DWORD                       dwShopScriptID;
        BOOL                        bCanRepair;
        BOOL                        bCoinShop;
        KNPC_SHOP_TEMPLATE_ITEMS    vecTemplateItems;
        DWORD                       dwRequireForceID;
    };

    typedef std::map<int, KNPC_SHOP_TEMPLATE> KNPC_SHOP_TEMPLATE_TABLE;
    KNPC_SHOP_TEMPLATE_TABLE m_NpcShopTemplateTable;

    BOOL LoadNpcShopTemplateItems(KNPC_SHOP_TEMPLATE* pShopTemplate, const char cszFileName[]);

public:
    KNPC_SHOP_TEMPLATE_ITEM* GetShopTemplateItem(DWORD dwShopTemplateID, int nItemTemplateIndex);
    int GetMentorItemScore(DWORD dwTabType, DWORD dwIndex);
    void SetMentorItemScore(DWORD dwTabType, DWORD dwIndex, int nContribution, int nExamPrint);

#ifdef _CLIENT
    BOOL UpDateShopInfo(DWORD dwTemplateID, DWORD dwShopID, int nShopType, DWORD dwValidPage, BOOL bCanRepair);
    BOOL LoadShopTemplate(DWORD dwShopTemplateID);
#endif

private:
    BOOL LoadReputationRebate();

    struct REPUTATION_REBATE
    {
        int nRebate;
        int nMaxRebate;
    };

    typedef map<uint64_t, REPUTATION_REBATE> REPUTATION_REBATE_MAP;
    REPUTATION_REBATE_MAP m_ReputationRebateMap;

public:
    int GetReputationRebate(DWORD dwForceID, int nReputeLevel);
    int GetMaxRebate(DWORD dwForceID, int nReputeLevel);
    BOOL CheckCoinShop(KNPC_SHOP_TEMPLATE* pShopTemplate);
    /* PORT-DEFERRED_WIRING[CALLER] owner=KRelayClient::OnCoinShopBuyItemRespond;
       target=0x080da2e8 -> KShopCenter::OnBuyCoinShopItem@0x081394d2; phase=PRE_BUILD. */
    BOOL OnBuyCoinShopItem(KPlayer* pPlayer, int nResult, DWORD dwTabType, DWORD dwIndex,
        int nRandomSeed, int nCount, int nCoinCost);
    
    void ClearShop();

private:
    typedef KMemory::KAllocator<std::pair<int64_t, int> > KMAP_ITEM_CONTRIBUTION_ALLOCATOR;
    typedef std::map<int64_t, int, std::less<int64_t>, KMAP_ITEM_CONTRIBUTION_ALLOCATOR> ITEM_CONTRIBUTION_MAP;
    ITEM_CONTRIBUTION_MAP m_ItemContributionMap;

    typedef KMemory::KAllocator<std::pair<DWORD, KShop*> > KMAP_COPY_TABLE_ALLOCTOR;
    typedef std::map<DWORD, KShop*, std::less<DWORD>, KMAP_COPY_TABLE_ALLOCTOR> KSHOP_LIST;
    KSHOP_LIST m_ShopList;
};
#endif //_INCLUDE_KSHOPCENTER_H_

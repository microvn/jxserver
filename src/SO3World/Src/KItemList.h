/************************************************************************/
/* 道具集合基类							                                */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.01.25	Create												*/
/************************************************************************/
#ifndef _KITEM_LIST_H_
#define _KITEM_LIST_H_

#include <vector>
#include <map>

#include "SO3Result.h"
#include "KItem.h"
#include "KInventory.h"

class KTarget;

#define TRADE_BOX_WIDTH		8
#define TRADE_BOX_HEIGHT	4

typedef struct 
{
	DWORD	dwBox;
	DWORD	dwX;
} TItemPos;

class KPlayer;
class KFEAEnchant;

struct KUSE_ITEM
{
    DWORD dwProfessionID;
    DWORD dwProfessionBranchID;
    DWORD dwProfessionLevel;
};

struct KSTART_USE_ITEM_PARAM 
{
    DWORD   dwOTAniID;      
    DWORD   dwEndAniID;     
    int     nFrames;        
    BOOL    bSetCD;         
    BOOL    bCastSkill;     
    char    szText[USE_ITEM_PROGRESS_NAME_LEN];        
};

class KItemList
{
public:
	int		m_nMoney;			//  金钱
	int		m_nMoneyLimit;		//  金钱上限
	int		m_nReserverd;		// target reserved state

    BOOL    m_bCubPackageOpened;
    BOOL    m_bBankOpened;      //  银行是否已打开

#ifdef _SERVER
    BOOL    m_bSoldListOpened;  //  回购列表是否已打开
    BOOL    m_bFinishLoadData;
#endif

    int     m_nEquipIDArray[3];

public:
	BOOL	Init(KPlayer* pPlayer);
	void    UnInit();

#ifdef _SERVER
    void    Activate();
#endif

    ADD_ITEM_RESULT_CODE    CanAddItem(KItem* pItem, BOOL bUseOnlySpecialBox = false);
    ADD_ITEM_RESULT_CODE    CanAddItemToPos(KItem* pItem, DWORD dwDestBox, DWORD dwDestX);
    ADD_ITEM_RESULT_CODE	AddItem(KItem* pItem, BOOL bNotifyClient = true);
	ADD_ITEM_RESULT_CODE	AddItemToPos(KItem* pItem, DWORD dwBoxIndex, DWORD dwX);

private:
	ADD_ITEM_RESULT_CODE	AddItemInternal(KItem* pItem, BOOL bNotifyClient = true, BOOL bUseOnlySpecialBox = false);

public:

    KItem*	PickUpItem(DWORD dwBoxIndex, DWORD dwX, BOOL bSync = true);
	KItem*	GetItem(DWORD dwBoxIndex, DWORD dwX);
    
    DWORD   GetItemAmountInEquipAndPackage(DWORD dwTabType, DWORD dwIndex, int nBookRecipeID = -1);
    DWORD	GetItemTotalAmount(DWORD dwTabType, DWORD dwIndex, int nBookRecipeID = -1);
    DWORD   GetItemAmountInBox(int nBoxIndex, DWORD dwTabType, DWORD dwIndex, int nBookRecipeID = -1);
    
    BOOL	GetItemPos(DWORD dwTabType, DWORD dwIndex, DWORD& dwBox, DWORD& dwX);
	BOOL	GetItemPos(DWORD dwID, DWORD& dwBox, DWORD& dwX);
    
    int		GetBoxType(DWORD dwBox);
	
#ifdef _SERVER
	BOOL	CostItemInEquipAndPackage(DWORD dwTabType, DWORD dwIndex, DWORD dwAmount, int nBookRecipeID = -1);
    BOOL    CostItemInAllBox(DWORD dwTabType, DWORD dwIndex, DWORD dwAmount, int nBookRecipeID = -1);
#endif
    //扣取某个指定物品的数量
    BOOL	CostSingleItem(DWORD dwBoxIndex, DWORD dwX, int nAmount);

    //查找指定大小的空闲空间
	BOOL	FindStackRoom(DWORD dwBoxIndex, DWORD& dwX, KItem* pItem);

#ifdef _SERVER
	BOOL	FindFreeSoldList(DWORD& dwX);
#endif

    BOOL	FindFreeRoomInPackage(
        DWORD&  dwBoxIndex, DWORD& dwX, 
        int     nItemGenerType = INVALID_CONTAIN_ITEM_TYPE, 
        int     nItemSubType = INVALID_CONTAIN_ITEM_TYPE
    );
	BOOL	FindStackRoomInPackage(DWORD& dwBoxIndex, DWORD& dwX, KItem* pItem);
	BOOL	FindStackRoomInBank(DWORD& dwBoxIndex, DWORD& dwX, KItem* pItem);
	
	int	    GetAllPackageFreeRoomSize(void);
	int     GetBoxFreeRoomSize(int nBoxIndex);

    //钱相关的方法
	int		GetMoney() { return m_nMoney; };
	BOOL	SetMoney(int nMoney);
    BOOL	AddMoney(int nMoney, BOOL bShowMsg = true);
    
    //钱上限相关的方法
	int		GetMoneyLimit() { return m_nMoneyLimit; };
	
	//交换两个物品的位置
    ITEM_RESULT_CODE    CanExchangeItemToPos(KItem* pItem, DWORD dwDestBox, DWORD dwDestX);
    ITEM_RESULT_CODE	CanExchangeItem(DWORD dwSrcBox, DWORD dwSrcX, DWORD dwDestBox, DWORD dwDestX);
    ITEM_RESULT_CODE	ExchangeItem(const TItemPos& Src, const TItemPos& Dest, DWORD dwAmount = 0);
    
    BOOL    GetEquipPos(int nRepresentIndex, DWORD *pdwEquipPos);
    EQUIPMENT_INVENTORY_TYPE GetEquipPosBySubType(EQUIPMENT_SUB_TYPE eSubType);

    ITEM_RESULT_CODE GetCanEquipPos(KItem* pItem, int* pnRetPos);

	ITEM_RESULT_CODE CanEquip(KItem* pItem, DWORD dwX);
    ITEM_RESULT_CODE CanUnEquip(DWORD dwX);

    BOOL	         Equip(KItem* pItem, DWORD dwX);
    BOOL	         UnEquip(DWORD dwX);

    //从身上删除一个物品，不从游戏世界删除掉
	ITEM_RESULT_CODE	DestroyItem(const TItemPos& Pos, BOOL bSync = true);
	ITEM_RESULT_CODE	DestroyItem(KItem* pItem, BOOL bSync = true);
    
    DWORD	GetBoxSize(DWORD dwBox);
    DWORD   GetBoxUsedSize(DWORD dwBox);
    DWORD   GetEmptyPos(DWORD dwBox);

#ifdef _SERVER
    BOOL    Load(BYTE* pbyData, size_t uDataLen);
    BOOL    Load(BYTE* pbyData, size_t uDataLen, int nVersion);
    BOOL    LoadItemList(BYTE* pbyData, size_t uDataLen, int nVersion);
    BOOL    LoadItemList_V0(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V1(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V2(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V3(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V4(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V5(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadItemList_V6(BYTE* pbyData, size_t uDataLen);
    BOOL    IsTimeLimitReturnItem(DWORD dwItemID);
    DWORD   GetTimeLimitReturnItemShopTemplateID(DWORD dwItemID);
    int     GetTimeLimitReturnItemShopItemIndex(DWORD dwItemID);
    BOOL    AddTimeLimitReturnItemInfo(
        DWORD dwItemID, DWORD dwShopTemplateID, int nShopItemIndex, time_t nEndTime
    );
    BOOL    DelTimeLimitReturnItemInfo(DWORD dwItemID);
    BOOL    DelTimeLimitSoldListInfo(DWORD dwItemID);
    void    TidyUpTimeLimitSoldList();
    BOOL    LoadTimeLimitReturnInfo_V0(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadTimeLimitReturnInfo(BYTE* pbyData, size_t uDataLen, int nVersion);
    BOOL    Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
	
    void    TidyUpSoldList();    
    void    AbradeArmor();
    void    AbradeItemAtEquipBox(DWORD dwPos);

    //按装备最大耐久度的百分比, 磨损一个包包中的所有装备(主要用在玩家复活时,对其的惩罚)
    BOOL    AbradeEquipment(int nBoxType, int nAbradePercent);
    void    UpdateItemID();
#endif

    BOOL	ApplyAttrib(KItem* pItem);
	BOOL	UnApplyAttrib(KItem* pItem);

    BOOL	IsPackageSlot(DWORD dwBox, DWORD dwX);
    BOOL    IsBankPackageSlot(DWORD dwBox, DWORD dwX);

#ifdef _SERVER
    BOOL	Repair(DWORD dwBox, DWORD dwX);
    BOOL	EnableBankPackage();
#endif
	int		GetBankPackageCount();

#ifdef _CLIENT
	BOOL	SetEnablePackageCount(int nCount);
#endif

    BOOL	IsBankPackageEnabled(EQUIPMENT_INVENTORY_TYPE eInventoryType);
#ifdef _SERVER
	BOOL	ConsumeAmmo(DWORD dwAmount);
	void	ReloadAmmo(DWORD dwTabType, DWORD dwIndex);

	ENCHANT_RESULT_CODE	AddEnchant(DWORD dwBoxIndex, DWORD dwX, DWORD dwEnchantID, KItem* pItem = NULL);
	ENCHANT_RESULT_CODE	RemoveEnchant(DWORD dwBoxIndex, DWORD dwX, int nMountIndex);
#endif
    ENCHANT_RESULT_CODE CanMountItem(DWORD dwSrcBox, DWORD dwSrcX, DWORD dwDestBox, DWORD dwDestX);
    ENCHANT_RESULT_CODE CanUnMountItem(DWORD dwBoxIndex, DWORD dwX, int nMountIndex);

    BOOL    SetEquipColor(DWORD dwBoxIndex, DWORD dwX, int nColorID);

    KInventory* GetAllInventory();
    KInventory* GetInventory(DWORD dwIndex);

    void    OnChangeCamp();

private:
    KItem*	FindItem(DWORD dwTabType, DWORD dwIndex, DWORD& dwBox, DWORD& dwX); 

#ifdef _SERVER
    void    CallItemScript(KItem* pItem, const TItemPos& crItemPos, KTarget& rTarget, BOOL& rbContinueSkill, BOOL& rbDestroyItem);
    USE_ITEM_RESULT_CODE CastItemSkill(KItem* pItem, KTarget& rTarget, BOOL* pbCostItemAtOnce);
#endif
    
    void    SetRepresentID(int nRepresentIndex, int nRepresentID);
    
    BOOL    GetEquipRepresentIndex(DWORD dwEquipPos, int* pnIndex);
    BOOL    GetEquipColorIndex(DWORD dwEquipPos, int* pnIndex);

	// 检测全身装备是否还符合需求
	BOOL	CheckEquipRequire(void);
	BOOL	PushItem(KItem* pItem, DWORD dwBox, BOOL bNotifyClient);
	ITEM_RESULT_CODE	ExchangePackageSlot(DWORD dwBox1, DWORD dwBox2);
	ITEM_RESULT_CODE	EnablePackage(DWORD dwBox, DWORD dwSize, int nContainGenerType, int nContainSubType);
	ITEM_RESULT_CODE	DisablePackage(DWORD dwBox);

	BOOL	ApplySetAttrib(DWORD dwSetID, int nSetNum);
	BOOL	UnApplySetAttrib(DWORD dwSetID, int nSetNum);
    
    BOOL    CostItemInBox(int nBoxIndex, DWORD dwTabType, DWORD dwIndex, DWORD& rdwAmount, int nBookRecipeID);
    
public:
	int		GetEquipedSetNum(DWORD dwSetID);

    USE_ITEM_RESULT_CODE CanUseItem(
        KItem* pItem, DWORD dwBox, KUSE_ITEM* pUseItemInfo, KTarget& rTarget
    );
    BOOL CheckUseItemTargetType(KItem* pItem);

#ifdef _SERVER
    USE_ITEM_RESULT_CODE UseDesignationItem(DWORD dwBox, DWORD dwX);
    USE_ITEM_RESULT_CODE UseItem(TItemPos ItemPos, KTarget& rTarget);
    USE_ITEM_RESULT_CODE UseItem(DWORD dwBox, DWORD dwX, KTarget& rTarget);
    
    void OnBreakUseItem(KItem* pItem);
#endif
    void FinishUseItem(KItem* pItem);

#ifdef _SERVER
private:
    USE_ITEM_RESULT_CODE UseItemScripts(KItem* pSrcItem, KTarget& rTarget);
    USE_ITEM_RESULT_CODE UseItemSkill(KItem* pSrcItem, KTarget& rTarget);
    USE_ITEM_RESULT_CODE UseItemScriptsAndSkill(KItem* pSrcItem, KTarget& rTarget);

    BOOL CallStartUseItemScripts(KItem* pSrcItem, KTarget& rTarget, KSTART_USE_ITEM_PARAM& rStartUseItemParam);
#endif
    
private:
	struct KDelayTradeInfo
	{
		DWORD  dwItemID;
		time_t nEndTime;
	};

	struct KTimeLimitReturnInfo
	{
		DWORD  dwShopTemplateID;
		int    nShopItemIndex;
		time_t nEndTime;
	};

	KInventory			m_Box[ibTotal];
	DWORD				m_dwBoxCount;									// 箱子的总个数
	int					m_nEnabledBankPackageCount;						// 激活的银行背包格子数

	KPlayer*			m_pPlayer;
    int                 m_nNextSoldListPos;
	KFEAEnchant*        m_pFEAEnchant;
	BOOL                m_bFEAActiveFlag[3];
	int                 m_nTotalEquipScore;
	int                 m_nTotalBaseScore;
	int                 m_nTotalStrengthScore;
	int                 m_nTotalMountsScore;
	int                 m_nTotalPackageSize;
	BOOL                m_bCangjianFlag;

public:
    // 开箱子
    DWORD               m_dwBoxItemID;
    std::vector<KItem*> m_BoxItem;

private:
	std::map<DWORD, DWORD> m_ItemChangeIDMap;
	std::map<unsigned long long, KDelayTradeInfo> m_DelayTradeMap;
	std::map<DWORD, KTimeLimitReturnInfo> m_TimeLimitReturnMap;
	std::map<DWORD, long> m_TimeLimitSoldListInfoMap;

public:
    
    void                ClearBox();
    BOOL                OpenBox(KItem* pBox);
    ADD_ITEM_RESULT_CODE CanLootBoxItem(DWORD dwBoxID);
#ifdef _SERVER
    BOOL                LootBoxItem(DWORD dwBoxIndex, DWORD dwPos);
#endif

};

DWORD g_GetBoxIndexFromSlot(DWORD dwSlot);

#endif	//_KITEM_LIST_H_

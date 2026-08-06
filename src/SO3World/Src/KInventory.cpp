#include "stdafx.h"
#include "KItem.h"
#include "KInventory.h"
#include "KSO3World.h"

BOOL KInventory::Init(DWORD dwSize)
{
	BOOL bResult = false;

	KGLOG_PROCESS_ERROR(dwSize <= MAX_ITEM_PACKAGE_SIZE);

	m_dwSize                 = dwSize;
    m_nContainItemGenerType  = INVALID_CONTAIN_ITEM_TYPE;
    m_nContainItemSubType    = INVALID_CONTAIN_ITEM_TYPE;
    m_nContainItemGenerTypeExtra = INVALID_CONTAIN_ITEM_TYPE;
    m_nContainItemSubTypeExtra   = INVALID_CONTAIN_ITEM_TYPE;

	ZeroMemory(m_pItemArray, sizeof(m_pItemArray));

	bResult = true;
Exit0:
	return bResult;
}

void KInventory::UnInit()
{
    for (int i = 0; i < (int)m_dwSize; i++)
    {
        if (m_pItemArray[i])
        {
            g_pSO3World->m_ItemSet.Unregister(m_pItemArray[i]);
            m_pItemArray[i]->UnInit();
            KMemory::Delete(m_pItemArray[i]);
            m_pItemArray[i] = NULL;
        }
    }
}

KItem* KInventory::GetItem(DWORD dwX)
{
	KItem* pItem = NULL;

	assert(m_dwSize <= MAX_ITEM_PACKAGE_SIZE);

	KGLOG_PROCESS_ERROR(dwX < m_dwSize);

	pItem = m_pItemArray[dwX];
Exit0:
	return pItem;
}

BOOL KInventory::PlaceItem(KItem* pItem, DWORD dwX)
{
	BOOL bResult = false;

	assert(m_dwSize <= MAX_ITEM_PACKAGE_SIZE);

	KGLOG_PROCESS_ERROR(pItem);
	KGLOG_PROCESS_ERROR(dwX < m_dwSize);
	KGLOG_PROCESS_ERROR(m_pItemArray[dwX] == NULL);
    
	m_pItemArray[dwX] = pItem;

	bResult = true;
Exit0:
	return bResult;
}

// 清除装备，返回被清的装备指针
KItem* KInventory::PickUpItem(DWORD dwX)
{
	KItem* pItem = NULL;

	assert(m_dwSize <= MAX_ITEM_PACKAGE_SIZE);
	KGLOG_PROCESS_ERROR(dwX < m_dwSize);

	pItem = m_pItemArray[dwX];
	m_pItemArray[dwX] = NULL;

Exit0:
	return pItem;
}

void KInventory::SetContainItemType(int nContainItemGenerType, int nContainItemSubType)
{
    m_nContainItemGenerType = nContainItemGenerType;
    m_nContainItemSubType   = nContainItemSubType;
    m_nContainItemGenerTypeExtra = INVALID_CONTAIN_ITEM_TYPE;
    m_nContainItemSubTypeExtra   = INVALID_CONTAIN_ITEM_TYPE;
}

void KInventory::GetContainItemType(int* pnContainItemGenerType, int* pnContainItemSubType)
{
    assert(pnContainItemGenerType);
    assert(pnContainItemSubType);

    *pnContainItemGenerType = m_nContainItemGenerType;
    *pnContainItemSubType   = m_nContainItemSubType;
}

BOOL KInventory::CheckContainItemType(int nItemGenerType, int nItemSubType)
{
    BOOL bPrimaryMatch = false;
    BOOL bExtraMatch   = false;

    if (m_nContainItemGenerType != INVALID_CONTAIN_ITEM_TYPE ||
        m_nContainItemGenerTypeExtra != INVALID_CONTAIN_ITEM_TYPE)
    {
        if (m_nContainItemGenerType != INVALID_CONTAIN_ITEM_TYPE)
        {
            if (m_nContainItemSubType == INVALID_CONTAIN_ITEM_TYPE)
            {
                if (m_nContainItemGenerType == nItemGenerType)
                    bPrimaryMatch = true;
            }
            else if (m_nContainItemGenerType == nItemGenerType &&
                     m_nContainItemSubType == nItemSubType)
            {
                bPrimaryMatch = true;
            }
        }

        if (m_nContainItemGenerTypeExtra != INVALID_CONTAIN_ITEM_TYPE)
        {
            if (m_nContainItemSubTypeExtra == INVALID_CONTAIN_ITEM_TYPE)
            {
                if (m_nContainItemGenerTypeExtra == nItemGenerType)
                    bExtraMatch = true;
            }
            else if (m_nContainItemGenerTypeExtra == nItemGenerType &&
                     m_nContainItemSubTypeExtra == nItemSubType)
            {
                bExtraMatch = true;
            }
        }

        if (!bPrimaryMatch && !bExtraMatch)
            return false;
    }

    return true;
}

BOOL KInventory::CheckContainItemType(KItem* pItem)
{
    assert(pItem);
    return CheckContainItemType(pItem->m_Common.nGenre, pItem->m_Common.nSub);
}

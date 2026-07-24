/* -------------------------------------------------------------------------
//	文件名		：	KObjAllocator.h
//	创建者		：	lailigao
//	创建时间	：	2005-1-20 22:28:43
//	功能描述	：	对象分配器-一次new多个对象，重复使用，避免频烦new/delete
//
// -----------------------------------------------------------------------*/
#ifndef __KOBJALLOCATOR_H__
#define __KOBJALLOCATOR_H__

#include "engine.h"

// -------------------------------------------------------------------------
template<class KObjT>
class KObjAllocator
{
private:
	KObjT*		m_pObj;
	int			m_nCount;
	KLinkArray	m_FreeIdx; //	可用表
	KLinkArray	m_UseIdx;  //	已用表
public:
	KObjAllocator() : m_pObj(NULL), m_nCount(0)
	{
	}
	~KObjAllocator()
	{
		delete []m_pObj;
	}
	BOOL Init(int nSize)
	{
		if (nSize <= 0)
			return FALSE;
		
		m_pObj = new KObjT[nSize];
		m_nCount = nSize;
		m_FreeIdx.Init(nSize);
		m_UseIdx.Init(nSize);

		int		i;
		// 开始时所有的数组元素都为空
		for (i = nSize - 1; i > 0; i--)
		{
			m_FreeIdx.Insert(i);
		}
		return TRUE;
	}

	KObjT* Malloc(BOOL bClear)
	{
		int i = m_FreeIdx.GetNext(0);
		if (i == 0)
			return NULL;

		m_FreeIdx.Remove(i);
		m_UseIdx.Insert(i);

		if (bClear)
			memset(&(m_pObj[i]), 0, sizeof(KObjT));

		//cprintf( "KObjAllocator::Malloc %d \n", i);
		return &(m_pObj[i]);
	}

	BOOL Free(KObjT* pObj)
	{
		if (NULL == pObj)
			return FALSE;

		int nIdx = pObj - m_pObj;
		if (nIdx < 0 || nIdx >= m_nCount)
			return FALSE;

		m_FreeIdx.Insert(nIdx);
		m_UseIdx.Remove(nIdx);

		//cprintf( "KObjAllocator::Free %d \n", nIdx);
		return TRUE;
	}
};

// -------------------------------------------------------------------------

#endif /* __KOBJALLOCATOR_H__ */

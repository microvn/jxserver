//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2004 by Kingsoft
// File:	KNode.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef	_ENGINE_KNODE_H_
#define	_ENGINE_KNODE_H_
//---------------------------------------------------------------------------

#include "debug.h"

#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

class KNode
{
public:
	KNode* m_pNext;
	KNode* m_pPrev;

public:
	KNode();
	virtual ~KNode(){};
	KNode*	GetNext();
	KNode*	GetPrev();
	void	InsertBefore(KNode* pNode);
	void	InsertAfter(KNode* pNode);
	void	Remove();
	int		IsLinked();
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// 函数:	Knode
// 功能:	构造
//---------------------------------------------------------------------------
inline KNode::KNode()
{
	m_pNext = NULL;
	m_pPrev = NULL;
}
//---------------------------------------------------------------------------
// 函数:	GetNext
// 功能:	下一个结点
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KNode::GetNext()
{
	if (m_pNext && m_pNext->m_pNext)
		return m_pNext;
	return NULL;
}
//---------------------------------------------------------------------------
// 函数:	GetPrev
// 功能:	前一个结点
// 返回:	KNode*
//---------------------------------------------------------------------------
inline KNode* KNode::GetPrev()
{
	if (m_pPrev && m_pPrev->m_pPrev)
		return m_pPrev;
	return NULL;
}
//----------------------------------------------------------------------------
// 函数:	InsertBefore
// 功能:	在前面插入一个结点
// 参数:	KNode*
//---------------------------------------------------------------------------
inline void KNode::InsertBefore(KNode *pNode)
{
	KASSERT(m_pPrev);
	if (!pNode || !m_pPrev)
		return;

	if(pNode->m_pNext || pNode->m_pPrev)
	{
		_ASSERT(0);
	}

	pNode->m_pPrev = m_pPrev;
	pNode->m_pNext = this;
	m_pPrev->m_pNext = pNode;
	m_pPrev = pNode;
}
//---------------------------------------------------------------------------
// 函数:	InsertAfter
// 功能:	在后面插入一个结点
// 参数:	KNode*
//---------------------------------------------------------------------------
inline void KNode::InsertAfter(KNode *pNode)
{
	KASSERT(m_pNext);
	if (!pNode || !m_pNext)
		return;
	pNode->m_pPrev = this;
	pNode->m_pNext = m_pNext;
	m_pNext->m_pPrev = pNode;
	m_pNext = pNode;
}
//---------------------------------------------------------------------------
// 函数:	Remove the node
// 功能:	删除这个结点
// 返回:	KNode*
//---------------------------------------------------------------------------
inline void KNode::Remove()
{
	KASSERT(m_pPrev);
	KASSERT(m_pNext);
	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	m_pPrev = NULL;
	m_pNext = NULL;
}
//---------------------------------------------------------------------------
// 函数:	IsLinked
// 功能:	这个节点是否链接
// 返回:	bool
//---------------------------------------------------------------------------
inline int KNode::IsLinked()
{
	return (m_pPrev && m_pNext);
}
//--------------------------------------------------------------------------------

#endif	//_ENGINE_KNODE_H_

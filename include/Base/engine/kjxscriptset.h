//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KJxScriptSet.h
// Date:	2001-10-18 11:56:34
// Code:	Romandou, lbh, fanzai
// Desc:	
//---------------------------------------------------------------------------
#ifndef KJxScriptSet_H
#define KJxScriptSet_H

#pragma once

#include "kjxscript.h"
#include "kstrbase.h"
#include "file.h"
#include <map>


enum _CODE_RESULT
{
	SSR_OK = 0,
	SSR_LOAD_ERROR, //加载脚本失败
	SSR_OVERFLOW,   //脚本已超过最大缓存数
};
//不以_CODE_RESULT直接做为返回结果，是因为0表示成功，与0为假的习惯不合，故定义SCRIPT_SET_RESULT重载bool
struct SCRIPT_SET_RESULT
{
	SCRIPT_SET_RESULT(_CODE_RESULT rs)
	{
		m_eResult = rs;
	}
	operator _CODE_RESULT()
	{
		return m_eResult;
	}
	operator bool()
	{
		return m_eResult == SSR_OK;
	}
	bool operator == (const _CODE_RESULT rs)const
	{
		return m_eResult == rs;
	}
	_CODE_RESULT m_eResult;
};

typedef time_t TIME_T;

struct SCRIPT_INFO
{
	bool m_bLoaded;
	KJxScript m_Script;
	char m_szFileName[MAX_PATH];
	TIME_T m_LastModifyTime;
};

typedef BOOL (*SCRIPT_HANDLE_FUN)(KJxScript*, LPCSTR);
class ENGINE_API KJxScriptSet
{
typedef std::map<DWORD, UINT>	SCRIPTNODESET;
typedef std::map<DWORD, UINT>::iterator	ITOR_SCRIPTNODE;
private:
	//加载某目录下含子目录的所有脚本（传入目录参数为绝对路径）
	SCRIPT_SET_RESULT _LoadScriptInDirectory(LPCSTR cszDirFullPath, BOOL bReloadExist = FALSE);
	//重新加载脚本文件，如之前未加载(包括预加载)，相当于LoadScript
	SCRIPT_SET_RESULT _ReloadScript(UINT index);
	DWORD GenID(LPCSTR cszScriptPath);
public:
	KJxScriptSet(int nSetSize);
	~KJxScriptSet();
public:
	//设置虚加载模式(即只记录文件名，不加载文件，到第一次GetScript时才加载)，默为为实加载模式
	void SetVirtualLoadMode(BOOL bMode);
	//加载一个脚本文件
	SCRIPT_SET_RESULT LoadScript(LPCSTR cszScriptPath, BOOL bReloadExist = FALSE);
	//加载某目录下含子目录的所有脚本（传入目录参数为相对路径）
	SCRIPT_SET_RESULT LoadScriptInDirectory(LPCSTR cszDirPath, BOOL bReloadExist = FALSE);
	//加载包伴侣文件中的列出的所有脚本
	SCRIPT_SET_RESULT LoadScriptInPackPartnerFile(LPCSTR cszPackPartnerFilePath, BOOL bReloadExist = FALSE);
	//重新加载脚本文件，如之前未加载(包括预加载)，相当于LoadScript
	SCRIPT_SET_RESULT ReloadScript(LPCSTR cszScriptPath);
	//重新加载所有脚本
	int  ReloadAllScript(BOOL bForce = FALSE); 

	//由文件ID查找并获取一个脚本，文件ID可由g_FileName2Id在外部构造(本不应如此但已成现实)
	KJxScript* GetScript(DWORD dwScriptId);
	//由文件名查找并获取一个脚本
	KJxScript* GetScript(LPCSTR cszScriptPath);
	//获取第一个脚本
	KJxScript* GetFirstScript();
	//获取下一个脚本
	KJxScript* GetNextScript(int nIndex);
	//由文件ID查找一个文件名
	const char *FindFile(DWORD dwScriptId);
	//清除所有脚本缓存
	void ClearAll();
	//设置Load脚本前的回调函数，不设或设为NULL则Load前不执行任何操作
	void SetPreLoadCallBack(SCRIPT_HANDLE_FUN fun);
	//逐个执行回调函数，当回调函数返回FALSE中断
	void ForEach(SCRIPT_HANDLE_FUN fun);
	//获取脚本总数
	UINT GetScriptCount();
protected:
	UINT m_dwCurrentScriptNum;
	SCRIPT_INFO *m_ScriptSet;
	int			m_nSetSize;
	SCRIPT_HANDLE_FUN	m_PreLoadCallBack;
	BOOL m_bVirtualLoadMode;
#pragma warning(disable: 4251)
	SCRIPTNODESET m_NodeSet;
#pragma warning(default: 4251)
};

inline void KJxScriptSet::SetPreLoadCallBack(SCRIPT_HANDLE_FUN fun)
{
	m_PreLoadCallBack = fun;
}

inline void KJxScriptSet::SetVirtualLoadMode(BOOL bMode)
{
	m_bVirtualLoadMode = bMode;
}

inline KJxScript* KJxScriptSet::GetFirstScript()
{
	if (m_dwCurrentScriptNum > 0)
		return &m_ScriptSet[0].m_Script;
	return NULL;
}

inline KJxScript* KJxScriptSet::GetNextScript(int nIndex)
{
	if (m_dwCurrentScriptNum > (UINT)nIndex + 1)
		return &m_ScriptSet[nIndex + 1].m_Script;
	return NULL;
}

inline void KJxScriptSet::ForEach(SCRIPT_HANDLE_FUN fun)
{
	DWORD i;
	for (i = 0; i < m_dwCurrentScriptNum; i++)
	{
		if (!fun(&m_ScriptSet[i].m_Script, m_ScriptSet[i].m_szFileName))
			break;
	}
}

inline DWORD KJxScriptSet::GenID(const char *cszScriptPath)
{
	char szRelativeName[MAX_PATH];
	strcpy(szRelativeName, cszScriptPath);
	g_StrLower(szRelativeName);
	return g_FileNameHash(szRelativeName);
}

inline KJxScript* KJxScriptSet::GetScript(const char *cszScriptPath)
{
	return GetScript(GenID(cszScriptPath));
}

inline UINT KJxScriptSet::GetScriptCount()
{
	return m_dwCurrentScriptNum;
}

#endif //KJxScriptSet_H

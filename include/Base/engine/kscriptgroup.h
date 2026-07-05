/* -------------------------------------------------------------------------
//	文件名		：	engine/kscriptgroup.h
//	创建者		：	fanzai
//	创建时间	：	2006-12-28
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __ENGINE_KSCRIPTGROUP_H__
#define __ENGINE_KSCRIPTGROUP_H__

// -------------------------------------------------------------------------
#include "kjxscript.h"
#include <map>

#pragma warning(disable: 4251)

class ENGINE_API KScriptGroup
{
private:
	enum _KE_FILESTATE
	{
		emKFILESTATE_NONE,
		emKFILESTATE_LOADING,
		emKFILESTATE_LOADED,
		emKFILESTATE_ERROR,
	};

	typedef std::map<DWORD, _KE_FILESTATE> _KMapFileState;

public:
	// 从脚本的全局变量中得到本类的指针
	static KScriptGroup* GetThis(KJxScript& sc);

	// 设置包伴侣文件，以后遍历读脚本的时候会自动使用这个伴侣遍历包内
	static BOOL SetPackPartnerFile(LPCSTR pszPackPartnerFilePath);

private:
	// 不重复地引用其它脚本
	static int _LuaInclude(KJxScript& sc);

	// 强行载入指定脚本，即使已经载入也会再载入一遍
	static int _LuaDoScript(KJxScript& sc);

public:
	KScriptGroup(LPCSTR pszGroupPath = NULL);
	~KScriptGroup()
	{ Clear(TRUE); }

	// 清除脚本缓存
	// bClearAll：TURE则将脚本环境完全清除（脚本内的全部变量都会清空），FALSE则只将所有文件置为未载入
	void Clear(BOOL bClearAll, BOOL bDebug = FALSE);

	// 加载目录下全部脚本文件
	BOOL LoadAllScript()
	{ return LoadScriptInDirectory(m_szGroupPath); }

	// 加载指定目录下全部脚本文件
	BOOL LoadScriptInDirectory(LPCSTR pszScriptPath);

	// 加载指定脚本，用于不使用LoadAll方式载入脚本的地方
	BOOL LoadScript(LPCSTR pszFileName, BOOL bFource = FALSE);

	// 获取脚本指针
	KJxScript* GetScript()
	{ return &m_scScript; }

	// 获取已加载脚本文件数
	DWORD GetFileCount() const
	{ return m_mapFileStates.size(); }

	// 设置是否可以加载包外文件（默认可以）
	BOOL SetCanUseUnpackFile(BOOL bCanUseUnpackFile)
	{ m_bCanUseUnpackFile = bCanUseUnpackFile; return TRUE; }

private:
	// 不重复加载指定脚本（传入目录参数为相对路径）
	BOOL _LoadScript(LPCSTR pszFileName);

	// 加载指定脚本，可能重复（传入目录参数为相对路径）
	BOOL _DoScript(LPCSTR pszFileName);

	// 加载某目录下含子目录的所有脚本（传入目录参数为绝对路径）
	BOOL _LoadScriptInDirectory(LPCSTR pszDirFullPath);

private:
	char			m_szGroupPath[MAX_PATH];
	KJxScript		m_scScript;
	_KMapFileState	m_mapFileStates;
	BOOL			m_bCanUseUnpackFile;
};

// -------------------------------------------------------------------------

#endif /* __ENGINE_KSCRIPTGROUP_H__ */

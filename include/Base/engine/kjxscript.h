/* -------------------------------------------------------------------------
//	文件名		：	engine/kjxscript.h
//	创建者		：	fanzai
//	创建时间	：	2006-12-28
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __ENGINE_KJXSCRIPT_H__
#define __ENGINE_KJXSCRIPT_H__

// -------------------------------------------------------------------------
#include <stdio.h>
#include <vector>
#include <stdarg.h>


#define KD_N_BASE_CALL_STACK	512		// 默认C使用的堆栈大小	注意！写循环很容易超！	TODO:FanZai	提高易用性！

struct lua_State;
class KJxScript;
class KLunaBase;

typedef int (*lua_CFunction) (lua_State *L);
typedef int (*KScriptFunction) (KJxScript &sc);

struct TScriptFunc
{
	LPCSTR			pszName;
	lua_CFunction	pfnFunc;
};

#define KD_SCRIPT_FUNDEF1(Name)				{#Name,		KJxScript::Conv2LuaFunc<Lua##Name>},
#define KD_SCRIPT_FUNDEF2(szName, fnFunc)	{szName,	KJxScript::Conv2LuaFunc<fnFunc>},

struct KSCRIPT_FUNCTION
{
	LPCSTR			pszName;
	KScriptFunction	pfnFunc;
	LPCSTR			pszHelpDesc;
	LPCSTR			pszParam;
	LPCSTR			pszHelpParam;
	LPCSTR			pszHelpReturn;
};

class ENGINE_API KJxScript
{
public:
	enum KE_DATA_TYPE
	{
		KE_DATA_TYPE_NONE = -1,
		KE_DATA_TYPE_NULL,
		KE_DATA_TYPE_BOOL,
		KE_DATA_TYPE_LUSER,
		KE_DATA_TYPE_NUMBER,
		KE_DATA_TYPE_STRING,
		KE_DATA_TYPE_TABLE,
		KE_DATA_TYPE_FUNCTION,
		KE_DATA_TYPE_USER,
		KE_DATA_TYPE_THREAD,
	};
	typedef int (*ScriptOutFun)(LPCSTR pszFormat, ...);

public:
	// 从脚本的全局变量中得到本类的指针
	static KJxScript* GetThis(lua_State* L);

	// 输出特定出错信息以及脚本调用堆栈，并不终止脚本执行
	static BOOL OutPutErrMsgV(lua_State* L, LPCSTR pszFormat, va_list vlist);
	// 输出特定出错信息以及脚本调用堆栈，并不终止脚本执行
	static BOOL OutPutErrMsgF(lua_State* L, LPCSTR pszFormat, ...)
	{
		va_list vlist;
		va_start(vlist, pszFormat);
		BOOL bOK	= OutPutErrMsgV(L, pszFormat, vlist);
		va_end(vlist);
		return bOK;
	}

	template <KScriptFunction pfnFunc>
	static int Conv2LuaFunc(lua_State *L)
	{
		KJxScript* pscSelf = NULL;
		KS_CHECK_BOOL(L);

		pscSelf	= GetThis(L);
		KS_CHECK_BOOL(pscSelf);

		return pfnFunc(*pscSelf);

	KS_EXIT:
		return 0;
	}

	// 释放脚本系统的空闲内存（针对所有KJxScript实例）
	static BOOL ReleaseAllFreeMemory();

private:
	// 统一的脚本指令控制函数
	static int _LuaFunction(lua_State *L);

	// 供Lua用的内存分配函数
	static LPVOID _Alloc_Fun(LPVOID pScript, LPVOID pMem, size_t nOldSize, size_t nNewSize);

public:
	static ScriptOutFun ms_pOutFun;
	static LPCSTR		ms_szOutSplit;
	static LPCSTR		ms_szOutEnd;

public:
	KJxScript(BOOL bInit = FALSE);
	~KJxScript(void);

	lua_State* GetLuaState()
	{ return m_pLuaState; }

	static void SetOutFun(ScriptOutFun pFun)
	{ ms_pOutFun = pFun; }

	static void SetOutFormat(LPCSTR pszOutSplit, LPCSTR pszOutEnd)
	{
		ms_szOutSplit	= pszOutSplit;
		ms_szOutEnd		= pszOutEnd;
	}

	// 初始化
	BOOL Init(BOOL bDebug = FALSE);

	// 释放
	void Exit();

	// 注册C函数
	BOOL RegisterGlobalFunction(LPCSTR pszFunName, lua_CFunction pfnFunc);
	BOOL RegisterTableFunctions(LPCSTR pszTableName, const TScriptFunc arFunc[], DWORD dwCount);
	BOOL RegisterTableFunctions(LPCSTR pszTableName, const KSCRIPT_FUNCTION* pFunc, DWORD dwCount);
	template <size_t _nCount>
	BOOL RegisterTableFunctions(LPCSTR pszTableName, const KSCRIPT_FUNCTION (&arFunc)[_nCount])
	{
		return RegisterTableFunctions(pszTableName, arFunc, (DWORD)_nCount);
	}

	// 载入定长字符串（以类似函数的形式放在栈顶，不执行）
	BOOL LoadBuffer(LPBYTE pbyBuffer, DWORD dwLen, LPCSTR pszFileName = NULL);
	// 载入字符串（以类似函数的形式放在栈顶，不执行）
	BOOL LoadBuffer(LPCSTR pszBuffer, LPCSTR pszFileName = NULL)
	{
		return LoadBuffer((LPBYTE)pszBuffer, strlen(pszBuffer), pszFileName);
	}

	// 函数调用保护，在需要较多次调用函数的地方一定要用
	BOOL SafeCallBegin(int &nBeginIndex);
	BOOL SafeCallEnd(int nBeginIndex);

	// 执行全局函数
	BOOL CallGlobalFunctionV(LPCSTR pszFunName, int nResults, LPCSTR pszFormat, va_list vlist);
	// 执行全局函数
	BOOL CallGlobalFunction(LPCSTR pszFunName, int nResults, LPCSTR pszFormat, ...)
	{
		va_list vlist;
		va_start(vlist, pszFormat);
		BOOL bOK	= CallGlobalFunctionV(pszFunName, nResults, pszFormat, vlist);
		va_end(vlist);
		return bOK;
	}
	// 执行Table下的指定函数
	BOOL CallTableFunctionV(LPCSTR pszTableName, LPCSTR pszFunName, int nResults, LPCSTR pszFormat, va_list vlist);
	BOOL CallTableFunction(LPCSTR pszTableName, LPCSTR pszFunName, int nResults, LPCSTR pszFormat, ...)
	{
		va_list vlist;
		va_start(vlist, pszFormat);
		BOOL bOK	= CallTableFunctionV(pszTableName, pszFunName, nResults, pszFormat, vlist);
		va_end(vlist);
		return bOK;
	}

	// 根据已经入栈的参数调用函数，并将函数、参数出栈，留下返回值
	BOOL DoCall(int nResults, int nParams);

	// 调用栈顶的函数，包含参数、返回值
	BOOL Execute(int nResults, LPCSTR pszFormat, ...);

	// 载入并执行字符串，无参数、返回值，堆栈安全
	BOOL DoBuffer(LPCSTR pszBuffer, LPCSTR pszFileName = NULL);
	// 载入并执行文件，无参数、返回值，堆栈安全
	BOOL DoFile(LPCSTR pszFileName, BOOL bPackOnly = FALSE);

	// 按照参数表检查
	BOOL CheckList(int nFromIdx, LPCSTR pszFormat)
	{
		KS_CHECK_BOOL(m_pLuaState);

		return _ReadList(nFromIdx, pszFormat) >= 0;

	KS_EXIT:
		return FALSE;
	}
	// 按照参数表读取，成功返回读取数量（>= 0），失败返回-1
	int ReadList(int nFromIdx, LPCSTR pszFormat, ...)
	{
		KS_CHECK_BOOL(m_pLuaState);

		{
			va_list vlist;
			va_start(vlist, pszFormat);
			BOOL bOK	= _ReadList(nFromIdx, pszFormat, vlist);
			va_end(vlist);
			return bOK;
		}

	KS_EXIT:
		return FALSE;
	}

	// 栈顶索引（也是栈的大小）
	int GetTopIndex();
	BOOL SetTopIndex(int nIndex);

	BOOL PopStack(int n) { return SetTopIndex(-n - 1); };
	
	// 判断数据类型
	KJxScript::KE_DATA_TYPE GetType(int nIndex);	// 获得数据类型
	BOOL IsNone(int nIndex)		{ return GetType(nIndex) == KE_DATA_TYPE_NONE; }
	BOOL IsNil(int nIndex)		{ return GetType(nIndex) == KE_DATA_TYPE_NULL; }
	BOOL IsBool(int nIndex)		{ return GetType(nIndex) == KE_DATA_TYPE_BOOL; }
	BOOL IsLUser(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_LUSER; }
	BOOL IsNumber(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_NUMBER; }
	BOOL IsString(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_STRING; }
	BOOL IsTable(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_TABLE; }
	BOOL IsFunction(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_FUNCTION; }
	BOOL IsUser(int nIndex)		{ return GetType(nIndex) == KE_DATA_TYPE_USER; }
	BOOL IsThread(int nIndex)	{ return GetType(nIndex) == KE_DATA_TYPE_THREAD; }

	// 读取栈中的值
	int			GetInt(int nIndex);							// 获得int
	BOOL		GetBool(int nIndex);						// 获得bool
	double		GetNum(int nIndex);							// 获得double
	LPCSTR		GetStr(int nIndex);							// 获得string
	LPCSTR		GetLStr(int nIndex, int& nLen);				// 获得string，同时获得长度
	KLunaBase*	GetObj(int nIndex);							// 获得c类的指针
	void*		GetUser(int nIndex);						// 获得user data (light、full)
	int			GetObjLen(int nIndex);						// 取得table的大小、string的长度
	BOOL		GetTableIndex(int nIndex, int i);			// 取得table的一个元素，并入栈
	BOOL		GetTableField(int nIndex, LPCSTR pszName);	// 取得table的一个元素，并入栈
	BOOL		GetGlobal(LPCSTR pszName);					// 取得全局变量，并入栈
	int			GetGlobalF(LPCSTR pszString);				// 含符号支持的取得全局变量，支持“.”和“:”。
															// 如果有“:”，会顺次将fun、table入栈，否则只入栈fun。

	// 按照参数表压入全部参数，成功返回压入数量（>= 0），失败返回-1
	int PushListV(LPCSTR pszFormat, va_list vlist)
	{
		KS_CHECK_BOOL(m_pLuaState);

		return _PushList(pszFormat, vlist);

	KS_EXIT:
		return FALSE;
	}
	int PushList(LPCSTR pszFormat, ...)
	{
		va_list vlist;
		va_start(vlist, pszFormat);
		BOOL bOK	= PushListV(pszFormat, vlist);
		va_end(vlist);
		return bOK;
	}
	// 压入新值
	BOOL PushNumber(double fValue);				// 压入double
	BOOL PushBool(BOOL bValue);					// 压入bool
	BOOL PushString(LPCSTR pszValue);			// 压入string
	BOOL PushLString(LPCSTR pszValue, int nLen);// 压入定长string
	BOOL PushStringF(LPCSTR pszFormat, ...);	// 压入格式化string
	BOOL PushObj(const KLunaBase* pLuna);		// 压入c类的指针
	BOOL PushPointer(const void* p);			// 压入任意指针
	BOOL PushNull();							// 压入nil
	BOOL PushTable();							// 压入table
	BOOL SetTableIndex(int nIndex);				// 将top的数据作为top-1的table的一个成员table[nIndex]，并将top出栈
	BOOL SetTableField(LPCSTR pszName);			// 将top的数据作为top-1的table的一个成员table[szName]，并将top出栈
	BOOL SetGlobalName(LPCSTR pszName);			// 将top的数据作为全局的一个变量名称，并将top出栈

	BOOL PushFromStack(int nIndex);				// 将栈中一个已有数据复制到栈顶
	BOOL RemoveFromStack(int nIndex);			// 将栈中一个已有数据移出
	BOOL MoveStack(int nIndex);					// 将当前栈顶的值插入到指定位置

	// 将另外一个脚本中的一段堆栈复制到当前脚本中
	BOOL CopyStack(KJxScript& sc, int nFromIdx, int nCount);

	// 输出特定出错信息以及脚本调用堆栈，并不终止脚本执行
	BOOL OutPutErrMsgV(LPCSTR pszFormat, va_list vlist)
	{ return OutPutErrMsgV(m_pLuaState, pszFormat, vlist); };
	// 输出特定出错信息以及脚本调用堆栈，并不终止脚本执行
	BOOL OutPutErrMsgF(LPCSTR pszFormat, ...)
	{
		va_list vlist;
		va_start(vlist, pszFormat);
		BOOL bOK	= OutPutErrMsgV(m_pLuaState, pszFormat, vlist);
		va_end(vlist);
		return bOK;
	}

	// 将栈中的一个值转换成可以储存、传送的Buffer
	// 返回值：
	//		>0	转换成功，返回值为实际使用Buffer长度
	//		0	转换失败，含有不可储存的数据类型
	//		-1	转换失败，Buffer长度不够
	// （注：Buffer不以'\0'结尾，且Buffer中可能含多个字符'\0'）
	int SaveValue2Buffer(LPBYTE pbyBuf, int nBufSize, int nIndex);

	// 将Buffer转换成一个值，放入栈顶（此Buffer由SaveValue2Buffer生成）
	// 转换失败不改变堆栈
	//（注：此函数应有安全设计，不应当因为传入Buffer非法而出现任何异常）
	BOOL LoadBuffer2Value(const BYTE* pbyBuf, int nBufSize);

	// 得到已分配内存大小
	UINT GetMemSize() CONST { return m_uMemSize; };
	// 得到已分配内存次数
	UINT GetAllocTimes() CONST { return m_uAllocTimes; };
	// 得到已分配的内存块的最大大小
	UINT GetBlockMaxSize() CONST { return m_uBlockMaxSize; };
	// 得到全部已分配的内存块大小
	UINT GetAllocMemSize() CONST;
	// 得到未使用的缓冲内存块大小
	UINT GetFreeMemSize() CONST;

protected:
	// 根据已经入栈的参数调用函数，并将函数、参数出栈，留下返回值
	BOOL _DoCall(int nResults, int nParams);

	// 含参数调用函数，并将函数、参数出栈，留下返回值
	BOOL _Execute(int nResults, BOOL bCallMethod, LPCSTR pszFormat, va_list vlist);

	// 根据参数表压入全部参数
	int _PushList(LPCSTR pszFormat, va_list vlist);

	// 压入一个Vector
	template <class TPushType, class TDataType>
	inline BOOL _PushVector(void (*pPushFun)(lua_State *, TPushType), const std::vector<TDataType> *pvArry);
	// 根据参数表压入参数，调用一次只压入一个值（有可能是一个table）
	BOOL _PushOne(LPCSTR& pszFormat, va_list& vlist);

	// 按照参数表检查/读取参数
	int _ReadList(int nFromIdx, LPCSTR pszFormat, va_list vlist = NULL);
	// 按照参数表检查/读取参数，调用一次只检查/读取一个值（有可能是一个table）
	BOOL _ReadOne(int nIndex, LPCSTR& pszFormat, va_list& vlist);

	// 载入文件（以类似函数的形式放在栈顶，不执行）
	BOOL _LoadFile(LPCSTR pszFileName, BOOL bPackOnly);

	// 将另外一个脚本中的一个值复制到当前脚本中
	BOOL _CopyStack(lua_State *L, int n);

	// 将栈顶的一个值转换成可以储存、传送的Buffer，并出栈
	int _SaveValue2Buffer(LPBYTE pbyBuf, int nBufSize);

	// 将栈顶的table转换成Buffer（供_SaveValue2Buffer调用，不出栈）
	int _SaveTable2Buffer(LPBYTE pbyBuf, int nBufSize);

	// 将Buffer转换成一个值，放入栈顶
	// 返回使用了的Buffer长度，0表示解析错误
	int _LoadBuffer2Value(const BYTE* pszBuf, int nBufSize);

	// 将Buffer（含'{'）转换成一个table，放入栈顶（供_LoadBuffer2Value调用）
	// 返回使用了的Buffer长度（包含'}'），0表示解析错误
	int _LoadBuffer2Table(const BYTE* pszBuf, int nBufSize);

private:
	// 检查Lua返回值，必要时显示出错信息
	BOOL _CheckRet(int nRet);
	// Lua出错输出
	static int _LuaError(lua_State *L);
	// Lua正常输出
	static int _LuaPrint(lua_State *L);

private:
	// 类对象不得拷贝
	KJxScript& operator = (const KJxScript&);
	KJxScript(KJxScript&);

protected:
	lua_State*	m_pLuaState;
	BOOL		m_bCanDebug;

private:
	UINT	m_uMemSize;			// 已分配内存大小
	UINT	m_uAllocTimes;		// 已分配内存次数
	UINT	m_uBlockMaxSize;	// 已分配的内存块的最大大小
};

// 脚本堆栈保护
class KScriptSafeCall
{
public:
	explicit KScriptSafeCall(KJxScript& sc)
		: m_rc(sc)
	{
		m_nTopIndex	= sc.GetTopIndex();
	}
	~KScriptSafeCall()
	{
		if (m_nTopIndex >= 0)
		{
			m_rc.SetTopIndex(m_nTopIndex);
			m_nTopIndex	= -1;
		}
	}

	INT GetBeginTopIndex() CONST
	{ return m_nTopIndex; }

protected:
	INT m_nTopIndex;
	KJxScript& m_rc;
};

// -------------------------------------------------------------------------

#endif /* __ENGINE_KJXSCRIPT_H__ */

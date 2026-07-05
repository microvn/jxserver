/************************************************************************/
/* Lua V5.02�ű�														*/
/* Copyright : Kingsoft 2006											*/
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2006.02.18	Create												*/
/* Comment	 :															*/
/*		��װ��Lua5�ű��Ĳ���											*/
/************************************************************************/

#ifndef _KLUA_SCRIPT_EX_H_
#define _KLUA_SCRIPT_EX_H_

#include <map>
#include <vector>

#include "recyclebin.h"

#include <lua5/lualib.h>
#include <lua5/lauxlib.h>
#include <engine/KLuaScriptEx.h>

#define MAX_CONST_LIST_COUNT	4

#define MAX_SCIRPT_NAME_LEN		256
#define MAX_ENV_NAME_LEN		16

#define LUA_SCRIPT_THIS		"KLuaScriptExThis"
#define LUA_SCRIPT_ID		"ScriptID"

struct KLuaFunc
{
	const char*		pszFuncName;
	lua_CFunction	pFunc;	
};

struct KLuaConst
{
	const char*		pszConstName;
	int				nValue;
};

struct KLuaConstList
{
	const char*		pszListName;
	KLuaConst*		pConstList;
};


struct KLuaScriptData
{
	char szName[MAX_SCIRPT_NAME_LEN];
	std::vector<DWORD> vecIncludeScriptID;
};

typedef void (*LUA_ERROR_HANDLER)(const char*);

interface ILuaScriptEx
{
	virtual void	Release() = 0;

	virtual BOOL	Init(void) = 0;
	virtual BOOL	UnInit(void) = 0;

	//����
	virtual DWORD	LoadFromFile(const char* pszFileName) = 0;
	virtual BOOL	LoadFromBuffer(DWORD dwScriptID, const char* pszScriptName, const char* pszBuffer, DWORD dwSize) = 0;

	//ע��C����,������
	virtual BOOL	RegisterFunction(KLuaFunc &LuaFunc) = 0;
	virtual BOOL	RegisterFunctions(KLuaFunc Funcs[], int nFuncCount = 0) = 0;
	virtual BOOL	RegisterConstList(KLuaConstList* pLuaConstList) = 0;

	//�������ýӿ�
	virtual void	SafeCallBegin(int* pIndex) = 0;
	virtual void	SafeCallEnd(int nIndex) = 0;
	virtual BOOL	IsFuncExist(DWORD dwScriptID, char* pszFuncName) = 0;
	virtual BOOL	CallFunction(DWORD dwScriptID, const char* pszFuncName, int nResults) = 0;
	virtual void	AddParamCount() = 0;		//���Ӳ����ļ���
	virtual BOOL	GetValuesFromStack(char* pszFormat , ...) = 0;
	virtual BOOL	GetValuesFromStack(char* pszFormat , va_list vlist) = 0;
	
	//��ȡLuaState
	virtual lua_State*	GetLuaState(void) = 0;

	//���ű��Ƿ����
	virtual BOOL		IsScriptExist(DWORD dwScriptID) = 0;

	//��ȡ��ǰ����ִ�еĽű�
	virtual DWORD		GetActiveScriptID() = 0;

	//�����·����ȡ�ű�ID
	virtual DWORD		ScriptNameToID(const char* pszName) = 0;

	//��ȡKLuaScriptData
	virtual KLuaScriptData* GetScriptData(DWORD dwScriptID) = 0;

	//ע��ErrorHandler
	virtual void				RegisterErrorHandler(LUA_ERROR_HANDLER pHandler) = 0;
	virtual LUA_ERROR_HANDLER	GetErrorHandler() = 0;

	template<class T> void PushValueToStack(T value);	
};

template<class T>
inline void ILuaScriptEx::PushValueToStack(T value)
{
	value->LuaGetObj(GetLuaState());
	AddParamCount();
}

template<>
inline void ILuaScriptEx::PushValueToStack(int value)
{
	lua_pushinteger(GetLuaState(), value);

	AddParamCount();
}

template<> 
inline void ILuaScriptEx::PushValueToStack(DWORD value)
{
	lua_pushinteger(GetLuaState(), (int)value);

	AddParamCount();
}

template<> 
inline void ILuaScriptEx::PushValueToStack(bool value)
{
	lua_pushboolean(GetLuaState(), value);

	AddParamCount();
}

template<> 
inline void ILuaScriptEx::PushValueToStack(BYTE value)
{
	lua_pushinteger(GetLuaState(), (int)value);

	AddParamCount();
}

template<>
inline void ILuaScriptEx::PushValueToStack(double value)
{
	lua_pushnumber(GetLuaState(), value);

	AddParamCount();
}

template<>
inline void ILuaScriptEx::PushValueToStack(char* value)
{
	lua_pushstring(GetLuaState(), value);

	AddParamCount();
}

template<>
inline void ILuaScriptEx::PushValueToStack(const char* value)
{
	lua_pushstring(GetLuaState(), value);

	AddParamCount();
}

template<>
inline void ILuaScriptEx::PushValueToStack(lua_CFunction value)
{
	lua_pushcfunction(GetLuaState(), value);

	AddParamCount();
}

class KLuaScriptEx : public ILuaScriptEx
{
public:
	KLuaScriptEx(void);
	~KLuaScriptEx(void);

	static KLuaScriptEx* New();
	void Release();

	//��ʼ��
	BOOL Init(void);
	//����ʼ��
	BOOL UnInit(void);

	//���ļ�������ű�
	//�����ķ���ֵ��BOOL�͸ĳ���DWORD,��ʾ�����ļ����ڲ�ID.
	DWORD LoadFromFile(const char* pszFileName);	
	//���ڴ�������ű�
	BOOL LoadFromBuffer(DWORD dwScriptID, const char* pszScriptName, const char* pszBuffer, DWORD dwSize);

	//ע��C����
	BOOL RegisterFunction(KLuaFunc &LuaFunc);
	//ע��C��������
	BOOL RegisterFunctions(KLuaFunc Funcs[], int nFuncCount = 0);
	//�ڽű�����ʱע��C����
	static BOOL RegisterFunctions(lua_State* L, KLuaFunc Funcs[], int nFuncCount = 0);
	//ע��ȫ�ֳ�����
	BOOL RegisterConstList(KLuaConstList* pLuaConstList);

	//ע������ص�����
	void RegisterErrorHandler(LUA_ERROR_HANDLER pHandler);
	LUA_ERROR_HANDLER GetErrorHandler();

	//���ýű�����
	BOOL CallFunction(DWORD dwScriptID, const char* pszFuncName, int nResults);
	void AddParamCount();

	void SafeCallBegin(int* pIndex);
	void SafeCallEnd(int nIndex);

	//�Ӷ�ջ��ȡֵ
	BOOL GetValuesFromStack(char* pszFormat , ...);
	BOOL GetValuesFromStack(char* pszFormat , va_list vlist);

	//ָ���ĺ����Ƿ��ڽű��д���
	BOOL IsFuncExist(DWORD dwScriptID, char* pszFuncName);

	//�趨ȫ������
	BOOL AddGlobalInteger(const char* pszVarName, int nValue);
	BOOL AddGlobalString(const char* pszValueName, const char* pszValue);

	//�����ļ��ֲ�����
	BOOL AddInteger(DWORD dwScriptID, const char* pszVarName, int nValue);
	BOOL AddString(DWORD dwScriptID, const char* pszValueName, const char* pszValue);

	//��ȡLua��ջ
	lua_State* GetLuaState(void);

	//�жϽű��Ƿ����
	BOOL IsScriptExist(DWORD dwScriptID);

	//��ȡ��ǰ���ڱ����õĽű�
	DWORD GetActiveScriptID();

	//��ȡScriptData
	KLuaScriptData* GetScriptData(DWORD dwScriptID);

	//�ѽű�·��ת����hash���ID
	DWORD ScriptNameToID(const char* pszName);

private:
	typedef std::map<DWORD, KLuaScriptData> SCRIPT_DATA_MAP;
	SCRIPT_DATA_MAP m_mapScriptData; 

	lua_State*			m_LuaState;
	LUA_ERROR_HANDLER	m_pErrorHandler;
	DWORD				m_dwActiveScript;
	int					m_nParamCount;		//��������ʱ��¼������������ʱ����

	static KRecycleBin<KLuaScriptEx, 16, RBAFM_NEW_DELETE> ms_Recycle;
	
	//ע��ű�include����
	static int Include(lua_State* L);		

	//��ֹ�������ڽű��б��޸�,����__newindex
	static int _NewIndexForConstList(lua_State* L);

	//����__index,Ϊ��ʵ���µ�Include
	static int _IndexForEnv(lua_State* L);

	//��_include����
	BOOL _GetVarInInclude(DWORD dwScriptID, const char* pszValueName);

	//������������
	BOOL _MakeEnv(const char* szEnvName);

#ifdef LUA_DEBUG
	static void LuaProcessDebugInfo(lua_State* L, lua_Debug* ar);
#endif //LUA_DEBUG
};

C_ENGINE_API ILuaScriptEx* g_CreateLuaScriptEx();

/* This source version calls the 2-arg factory (allocator callback + context),
 * exported extern-C by libEngine_Lua5D.so. */
extern "C" ILuaScriptEx* CreateLuaInterface(
	void* (*pfnAlloc)(void*, void*, unsigned int, unsigned int), void* pvContext);

#endif	//_KLUA_SCRIPT_EX_H_

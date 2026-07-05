/************************************************************************/
/* C Class -> Lua Class 映射											*/
/* Copyright : Kingsoft 2006										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2006.02.28	Create												*/
/*		2006.11.29	FanZai修改											*/
/************************************************************************/
#ifndef _LUNA_H_
#define _LUNA_H_

#include "kjxscript.h"

//映射相关宏定义

//申明该类需要导出变量或者函数给Lua使用
#define __DECLARE_LUA_CLASS(__CLASS_NAME__)								\
	typedef void (__CLASS_NAME__::*VoidFunc)(void);						\
	static const char s_szClassName[];									\
	static KLuaData s_LuaInterface[];									\
	static int LuaDispatcher(lua_State* L)								\
	{ return _LuaDispatcher(L, s_szClassName); };						\
	static int LuaTostring(lua_State* L)								\
	{ return _LuaTostring(L, s_szClassName); };							\
	static int LuaIndex(lua_State* L)									\
	{ return _LuaIndex(L, s_szClassName, LuaDispatcher); };				\
	static int LuaNewIndex(lua_State* L)								\
	{ return _LuaNewIndex(L, s_szClassName); };							\
	static int LuaGetDataInfo(lua_State* L)								\
	{ return _LuaGetDataInfo(L, s_szClassName); };						\
	static BOOL Register(KJxScript& sc)									\
	{																	\
		return _Register(sc.GetLuaState(), s_szClassName,				\
						s_LuaInterface, LuaTostring, LuaIndex,			\
						LuaNewIndex, LuaGetDataInfo);					\
	}																	\

#define DECLARE_LUA_CLASS(__CLASS_NAME__)								\
	__DECLARE_LUA_CLASS(__CLASS_NAME__);								\
	BOOL PushCObj(lua_State* L) const									\
	{ return _PushCObj(L, s_szClassName); };							\

#define DECLARE_LUA_CLASS_COPYMODE(__CLASS_NAME__)						\
	__DECLARE_LUA_CLASS(__CLASS_NAME__);								\
	BOOL PushCObj(lua_State* L) const									\
	{ return _PushCObj(L, s_szClassName, sizeof(__CLASS_NAME__)); };	\

/************************************************************************/

//申明导出该整形变量
#define DECLARE_LUA_STRUCT_INTEGER(__VAR_NAME__, __REAL_NAME__)					\
	int get##__VAR_NAME__(void) const {return __REAL_NAME__;};					\
	void set##__VAR_NAME__(int nValue)	{__REAL_NAME__ = nValue;}

//申明导出该无符号整形变量
#define DECLARE_LUA_STRUCT_DWORD(__VAR_NAME__, __REAL_NAME__)					\
	DWORD get##__VAR_NAME__(void) const {return __REAL_NAME__;};				\
	void set##__VAR_NAME__(DWORD dwValue)	{__REAL_NAME__ = dwValue;}

//申明导出该字符串
#define DECLARE_LUA_STRUCT_STRING(__VAR_NAME__, __STR_LEN__, __REAL_NAME__)		\
	LPCSTR get##__VAR_NAME__(void) const {return __REAL_NAME__;};				\
	void set##__VAR_NAME__(LPCSTR pszValue)										\
	{strncpy((__REAL_NAME__), pszValue, __STR_LEN__);							\
	(__REAL_NAME__)[(__STR_LEN__) - 1] = 0;}

//申明导出该枚举型变量
#define DECLARE_LUA_STRUCT_ENUM(__VAR_NAME__, __REAL_NAME__)					\
	int get##__VAR_NAME__(void) const {return (int)(__REAL_NAME__);};			\
	void set##__VAR_NAME__(int nValue)	{*(int*)&(__REAL_NAME__) = nValue;	}	// TODO: 这样不安全 by FanZai

//申明导出该布尔型变量
#define DECLARE_LUA_STRUCT_BOOL(__VAR_NAME__, __REAL_NAME__)					\
	int get##__VAR_NAME__(void) const {return (int)(__REAL_NAME__);};			\
	void set##__VAR_NAME__(int nValue)	{*(int*)&(__REAL_NAME__) = nValue;}		// TODO: 这样不安全 by FanZai

/************************************************************************/

#define DECLARE_LUA_INTEGER(__VAR_NAME__)	DECLARE_LUA_STRUCT_INTEGER(__VAR_NAME__, m_n##__VAR_NAME__)
#define DECLARE_LUA_DWORD(__VAR_NAME__)		DECLARE_LUA_STRUCT_DWORD(__VAR_NAME__, m_dw##__VAR_NAME__)
#define DECLARE_LUA_STRING(__VAR_NAME__)	DECLARE_LUA_STRUCT_STRING(__VAR_NAME__, sizeof(m_sz##__VAR_NAME__), m_sz##__VAR_NAME__)
#define DECLARE_LUA_ENUM(__VAR_NAME__)		DECLARE_LUA_STRUCT_ENUM(__VAR_NAME__, m_e##__VAR_NAME__)
#define DECLARE_LUA_BOOL(__VAR_NAME__)		DECLARE_LUA_STRUCT_BOOL(__VAR_NAME__, m_dw##__VAR_NAME__)

/************************************************************************/

//开始定义导出类
#define DEFINE_LUA_CLASS_BEGIN(__CLASS_NAME__)									\
	typedef __CLASS_NAME__	__TClass;											\
	const char __TClass::s_szClassName[] = "_"#__CLASS_NAME__;					\
	KLunaBase::KLuaData __TClass::s_LuaInterface[] = {

/************************************************************************/

//注册整形变量
#define REGISTER_LUA_INTEGER(__VAR_NAME__, __VAR_DESC__)						\
	KLunaBase::_MkLuaValueInt("n"#__VAR_NAME__, __VAR_DESC__,					\
		&__TClass::set##__VAR_NAME__, &__TClass::get##__VAR_NAME__),

//注册无符号整形变量
#define REGISTER_LUA_DWORD(__VAR_NAME__, __VAR_DESC__)							\
	KLunaBase::_MkLuaValueDword("dw"#__VAR_NAME__, __VAR_DESC__,				\
		&__TClass::set##__VAR_NAME__, &__TClass::get##__VAR_NAME__),

//注册字符串变量
#define REGISTER_LUA_STRING(__VAR_NAME__, __VAR_DESC__)							\
	KLunaBase::_MkLuaValueStr("sz"#__VAR_NAME__, __VAR_DESC__,					\
		&__TClass::set##__VAR_NAME__, &__TClass::get##__VAR_NAME__),

//注册枚举型变量
#define REGISTER_LUA_ENUM(__VAR_NAME__, __VAR_DESC__)							\
	KLunaBase::KLuaData("e"#__VAR_NAME__, KLunaBase::ldtEnum, __VAR_DESC__,		\
		(KLunaBase::SetIntFunc)(&__TClass::set##__VAR_NAME__),					\
		(KLunaBase::GetIntFunc)(&__TClass::get##__VAR_NAME__)),

//注册布尔型变量
#define REGISTER_LUA_BOOL(__VAR_NAME__, __VAR_DESC__)							\
	KLunaBase::KLuaData("b"#__VAR_NAME__, KLunaBase::ldtBool, __VAR_DESC__,		\
		(KLunaBase::SetIntFunc)(&__TClass::set##__VAR_NAME__),					\
		(KLunaBase::GetIntFunc)(&__TClass::get##__VAR_NAME__)),

/************************************************************************/

//注册只读整形变量
#define REGISTER_LUA_INTEGER_READONLY(__VAR_NAME__, __VAR_DESC__)				\
	KLunaBase::_MkLuaValueInt("n"#__VAR_NAME__, __VAR_DESC__,					\
		(void (__TClass::*)(int))NULL, &__TClass::get##__VAR_NAME__),

//注册只读无符号整形变量
#define REGISTER_LUA_DWORD_READONLY(__VAR_NAME__, __VAR_DESC__)					\
	KLunaBase::_MkLuaValueDword("dw"#__VAR_NAME__, __VAR_DESC__,				\
		(void (__TClass::*)(DWORD))NULL, &__TClass::get##__VAR_NAME__),

//注册只读字符串变量
#define REGISTER_LUA_STRING_READONLY(__VAR_NAME__, __VAR_DESC__)				\
	KLunaBase::_MkLuaValueStr("sz"#__VAR_NAME__, __VAR_DESC__,					\
		(void (__TClass::*)(LPCSTR))NULL, &__TClass::get##__VAR_NAME__),

//注册只读枚举型变量
#define REGISTER_LUA_ENUM_READONLY(__VAR_NAME__, __VAR_DESC__)					\
	KLunaBase::KLuaData("e"#__VAR_NAME__, KLunaBase::ldtEnum, __VAR_DESC__,		\
		(KLunaBase::VoidFunc)NULL,												\
		(KLunaBase::GetIntFunc)(&__TClass::get##__VAR_NAME__)),

//注册只读布尔型变量
#define REGISTER_LUA_BOOL_READONLY(__VAR_NAME__, __VAR_DESC__)					\
	KLunaBase::KLuaData("b"#__VAR_NAME__, KLunaBase::ldtBool, __VAR_DESC__,		\
		(KLunaBase::VoidFunc)NULL,												\
		(KLunaBase::GetIntFunc)(&__TClass::get##__VAR_NAME__)),


/************************************************************************/

//注册导出函数
#define REGISTER_LUA_FUNC(__FUNC_NAME__, __FUNC_DESC__, __FUNC_PARAM__,			\
							__PARAM_DESC__, __RETURN_DESC__)					\
	KLunaBase::_MkLuaFunc(#__FUNC_NAME__, __FUNC_DESC__,						\
		&__TClass::Lua##__FUNC_NAME__, __FUNC_PARAM__,							\
		__PARAM_DESC__, __RETURN_DESC__),

/************************************************************************/

//定义导出类结束
#define DEFINE_LUA_CLASS_END()													\
		KLunaBase::KLuaData()													\
	};

//辅助基类	不再使用模版，目的是可以把部分代码转移到.cpp中
class ENGINE_API KLunaBase
{
public:
	//Lua导出的数据类型，仅内部使用
	enum LUA_DATA_TYPE
	{
		ldtInvalid = 0,

		ldtInteger,
		ldtString,
		ldtDword,
		ldtEnum,
		ldtBool,
		ldtFunc,
		ldtClass,

		ldtTotal
	};

	typedef void (KLunaBase::*VoidFunc)(void) const;

	typedef void (KLunaBase::*SetIntFunc)(int);
	typedef int (KLunaBase::*GetIntFunc)(void) const;

	typedef void (KLunaBase::*SetStrFunc)(LPCSTR);
	typedef LPCSTR (KLunaBase::*GetStrFunc)(void) const;

	typedef void (KLunaBase::*SetDwordFunc)(DWORD);
	typedef DWORD (KLunaBase::*GetDwordFunc)(void) const;

	typedef int (KLunaBase::*LuaFunc)(KJxScript& sc);

	struct KLuaData
	{
		LPCSTR			pszName;		//成员名称
		LUA_DATA_TYPE	eType;			//成员类型
		LPCSTR			pszDesc;		//文字描述
		union
		{
			struct 
			{
				VoidFunc	fnSet;			//数据写接口函数
				VoidFunc	fnGet;			//数据读接口函数
				DWORD		dwReserve1;		// 保留字段
				DWORD		dwReserve2;		// 保留字段
			}data;
			struct 
			{
				LuaFunc		fnLuaFunc;		//调用的函数
				LPCSTR		pszParam;		//入口参数
				LPCSTR		pszParamDesc;	//入口参数文字描述
				LPCSTR		pszReturnDesc;	//返回值的文字描述
			}func;
		};

		// 导出变量
		KLuaData(LPCSTR pszDataName, LUA_DATA_TYPE eDataType, LPCSTR pszDataDesc, VoidFunc fnSet, VoidFunc fnGet)
		{
			pszName			= pszDataName;
			eType			= eDataType;
			pszDesc			= pszDataDesc;
			data.fnSet		= fnSet;
			data.fnGet		= fnGet;
			data.dwReserve1	= 0;
			data.dwReserve2	= 0;
		}
		// 导出函数
		KLuaData(LPCSTR pszFuncName, LPCSTR pszFuncDesc, LuaFunc fnLunFunc, LPCSTR pszParam,
					LPCSTR pszParamDesc, LPCSTR pszReturnDesc)
		{
			pszName				= pszFuncName;
			eType				= ldtFunc;
			pszDesc				= pszFuncDesc;
			func.fnLuaFunc		= fnLunFunc;
			func.pszParam		= pszParam;
			func.pszParamDesc	= pszParamDesc;
			func.pszReturnDesc	= pszReturnDesc;
		}
		// 导出结束
		KLuaData()
		{
			ZeroStruct(*this);
		}
	};

	// 导出变量
	template <class _TClass_>
	static KLuaData _MkLuaValueInt(LPCSTR pszDataName, LPCSTR pszDataDesc,
								void (_TClass_::*fnSet)(int), int (_TClass_::*fnGet)(void) const)
	{ return KLuaData(pszDataName, ldtInteger, pszDataDesc, (VoidFunc)fnSet, (VoidFunc)fnGet); }

	template <class _TClass_>
	static KLuaData _MkLuaValueDword(LPCSTR pszDataName, LPCSTR pszDataDesc,
								void (_TClass_::*fnSet)(DWORD), DWORD (_TClass_::*fnGet)(void) const)
	{ return KLuaData(pszDataName, ldtDword, pszDataDesc, (VoidFunc)fnSet, (VoidFunc)fnGet); }

	template <class _TClass_>
	static KLuaData _MkLuaValueStr(LPCSTR pszDataName, LPCSTR pszDataDesc,
								void (_TClass_::*fnSet)(LPCSTR), LPCSTR (_TClass_::*fnGet)(void) const)
	{ return KLuaData(pszDataName, ldtString, pszDataDesc, (VoidFunc)fnSet, (VoidFunc)fnGet); }

	// 导出函数
	template <class _TClass_>
	static KLuaData _MkLuaFunc(LPCSTR pszFuncName, LPCSTR pszFuncDesc, int (_TClass_::*fnLunFunc)(KJxScript&),
						LPCSTR pszParam, LPCSTR pszParamDesc, LPCSTR pszReturnDesc)
	{ return KLuaData(pszFuncName, pszFuncDesc, (LuaFunc)fnLunFunc, pszParam, pszParamDesc, pszReturnDesc); }

	virtual BOOL PushCObj(lua_State* L) const	= 0;
	static KLunaBase* GetCObj(lua_State* L, int nIndex);

private:
	static const KLuaData* GetLuaData(lua_State* L, int nIndex);

protected:
	BOOL _PushCObj(lua_State* L, LPCSTR pszClassName, int nSize = 0) const;

	static BOOL _Register(lua_State* L, LPCSTR pszClassName, KLuaData* pLuaData,
		lua_CFunction pFunTos, lua_CFunction pFunIdx, lua_CFunction pFunNew, lua_CFunction pFunInf) ;

	static int _LuaGetDataInfo(lua_State* L, LPCSTR pszClassName);

	// 导出函数分派器，可以将static的调用，转换成有this指针的类成员函数调用
	static int _LuaDispatcher(lua_State *L, LPCSTR pszClassName);

	// 重载字符串转换函数
	static int _LuaTostring (lua_State* L, LPCSTR pszClassName);

	// 重载GetTable操作
	static int _LuaIndex(lua_State* L, LPCSTR pszClassName, lua_CFunction pFunDis);

	// 重载SetTable操作
	static int _LuaNewIndex(lua_State* L, LPCSTR pszClassName);

	// 方便脚本操作的临时Table
	static BOOL GetScriptTempTable(lua_State* L, LPCSTR pszClassName, int nIndex);
	static BOOL ClearScriptTempTable(lua_State* L, LPCSTR pszClassName, int nIndex);

private:
	struct _LuaLunaData 
	{
		BYTE byType;
		const void* pData;
	};

	enum _KE_LUNADATA_TYPE
	{
		emKLUNADATA_TYPE_POINTER,
		emKLUNADATA_TYPE_COPY,
	};
};

#endif	//_LUNA_H_

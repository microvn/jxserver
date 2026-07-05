/* -------------------------------------------------------------------------
//	文件名		：	kcolor.h
//	创建者		：	FanZai
//	创建时间	：	2007-12-18 20:44:54
//	功能描述	：	色彩值管理，暂未加入多线程安全
//
// -----------------------------------------------------------------------*/
#ifndef __KCOLOR_H__
#define __KCOLOR_H__

// -------------------------------------------------------------------------

// 为了方便使用的颜色值储存结构
// 注：Alpha值0xff是不透明，0是全透明。注意避免因省略Alpha值而导致的未预期的全透明。
union KCOLOR
{
	DWORD dwColorCode;
	struct  
	{
		BYTE byBlue, byGreen, byRed, byAlpha;
	};

	KCOLOR() : dwColorCode((DWORD)-1) { }
	KCOLOR(DWORD dwColor) : dwColorCode(dwColor) { }
	KCOLOR(BYTE byRed_, BYTE byGreen_, BYTE byBlue_, BYTE byAlpha_ = 0xff) :
	byRed(byRed_), byGreen(byGreen_), byBlue(byBlue_), byAlpha(byAlpha_) { }

	operator DWORD() CONST { return dwColorCode; }
	DWORD operator =(DWORD dwColor) { return dwColorCode = dwColor; }
};

// 颜色名对照表元素
struct KCOLORTABLE
{
	CHAR szName[32];	// 颜色名（必须非数字开头）
	KCOLOR unColor;
};

//增加一个unicode版本, 暂时不全改造unicode

struct KCOLORTABLEW
{
	WCHAR szName[32];
	KCOLOR unColor;
};


extern "C"
{
	ENGINE_API	DWORD	_g_GetColorValue(LPCSTR pszColor, DWORD dwDefault);	// 内部实现。对外接口见g_GetColorValue。
	ENGINE_API	LPCSTR	g_FindColorName(KCOLOR unColor);					// 根据颜色值查找其名字

	ENGINE_API	BOOL	g_SetColorTable(CONST KCOLORTABLE* psColorTab, INT nCount);	// 设定颜色对应表（会清空原有表）
	ENGINE_API	INT		g_GetColorTableCount();										// 得到颜色表元素数量
	ENGINE_API	BOOL	g_GetColorTable(INT nIndex, KCOLORTABLE& rsColorTab);		// 得到一个颜色表元素
	ENGINE_API	BOOL	g_SetDefaultColorTable();									// 默认颜色表（为了兼容无自定义）


	ENGINE_API	DWORD	_g_GetColorValueW(LPCWSTR pszColor, DWORD dwDefault);	// 内部实现
	ENGINE_API	LPCWSTR	 g_FindColorNameW(KCOLOR unColor);						// 根据颜色值查找其名字
	ENGINE_API	BOOL	g_SetColorTableW(CONST KCOLORTABLEW* psColorTab, INT nCount);	// 设定颜色对应表（会清空原有表）
	ENGINE_API	INT		g_GetColorTableCountW();	
	ENGINE_API	BOOL	g_GetColorTableW(INT nIndex, KCOLORTABLEW& rsColorTab);		// 得到一个颜色表元素
	ENGINE_API	BOOL	g_SetDefaultColorTableW();									// 默认颜色表（为了兼容无自定义）
}

// 获得字符串表示的颜色值，字符串可以用以下三种方式表达：
//	1、颜色名（必须非数字开头）
//	2、颜色编码（0xAARRGGBB，字符串长度小于等于8则认为省略AA，默认0xff不透明）
//	3、R,G,B[,A] 表达式（逗号分开，Alpha可选）
inline KCOLOR g_GetColorValue(LPCSTR pszColor, KCOLOR unDefault = KCOLOR())
{
	return _g_GetColorValue(pszColor, unDefault);
}

//---------------------------------------------------------------------------

inline KCOLOR g_GetColorValueW(LPCWSTR pszColor, KCOLOR unDefault = KCOLOR())
{
	return _g_GetColorValueW(pszColor, unDefault);
}

// -------------------------------------------------------------------------

#endif /* __KCOLOR_H__ */

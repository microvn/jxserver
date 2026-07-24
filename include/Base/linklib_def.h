/* -------------------------------------------------------------------------
//	文件名		：	linklib_def.h
//	创建者		：	llg
//	创建时间	：	2007-12-19 10:53:17
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
// #ifndef __LINKLIB_DEF_H__
// #define __LINKLIB_DEF_H__


#ifndef __INCLUDE_KSBASE_H__
#	include <ksdef.h>
#endif

#undef _LINKLIB_SUFFIX
#undef _LINKLIB_PURE
#undef LINKLIB_SUFFIX

// -------------------------------------------------------------------------
#if defined(_DEBUG)
#	define _LINKLIB_SUFFIX	"d.lib"
#else
#	define _LINKLIB_SUFFIX	".lib"
#endif

// 是否使用纯Lib(非DLL)
#if defined(KD_USE_PURELIB)
#	define _LINKLIB_PURE	"lib"
#else
#	define _LINKLIB_PURE
#endif // #if defined(KD_USE_PURELIB)

#define LINKLIB_SUFFIX		_LINKLIB_PURE _LINKLIB_SUFFIX

#ifndef LINKLIB_MSG
#	define LINKLIB_MSG		"\t程序将自动链接模块 - "
#endif // #ifndef LINKLIB_MSG	
// -------------------------------------------------------------------------

// #endif /* __LINKLIB_DEF_H__ */

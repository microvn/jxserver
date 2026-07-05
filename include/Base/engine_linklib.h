/* -------------------------------------------------------------------------
//	文件名		：	engine_linklib.h
//	创建者		：	llg
//	创建时间	：	2007-12-19 9:31:39
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __ENGINE_LINKLIB_H__
#define __ENGINE_LINKLIB_H__

/// -------------------------------------------------------------------------
#include "linklib_def.h"

#if defined(KD_SCRIPT_JX)
#	define LINKLIB_NAME		"engine"// "engine_jxscript"
#elif defined(LUA_V_5)
#	define LINKLIB_NAME		"engine_lua5"
#else
#	define LINKLIB_NAME		"engine"
#endif

#define LINKLIB				LINKLIB_NAME LINKLIB_SUFFIX
// -------------------------------------------------------------------------

#if !defined(__Linked_engine)
#	define __Linked_engine
#	pragma message(LINKLIB_MSG LINKLIB)
#	pragma linklib(LINKLIB)
#endif

#undef LINKLIB_NAME
#undef LINKLIB

// -------------------------------------------------------------------------

#endif /* __ENGINE_LINKLIB_H__ */

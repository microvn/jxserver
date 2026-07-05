/* -------------------------------------------------------------------------
//	文件名		：	common_linklib.h
//	创建者		：	lailigao
//	创建时间	：	2004-7-2 9:14:20
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __COMMON_LINKLIB_H__
#define __COMMON_LINKLIB_H__

// -------------------------------------------------------------------------
#include "linklib_def.h"

#define	LINKLIB_NAME	"common"
#define	LINKLIB			LINKLIB_NAME LINKLIB_SUFFIX
// -------------------------------------------------------------------------

#if !defined(__Linked_base_net_common)
#	define __Linked_base_net_common
#	pragma message(LINKLIB_MSG LINKLIB)
#	pragma linklib(LINKLIB)
#endif

#undef	LINKLIB_NAME
#undef LINKLIB
// -------------------------------------------------------------------------

#endif /* __COMMON_LINKLIB_H__ */

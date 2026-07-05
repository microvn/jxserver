/* -------------------------------------------------------------------------
//	文件名		：	rainbow_linklib.h
//	创建者		：	llg
//	创建时间	：	2007-12-19 10:26:17
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __RAINBOW_LINKLIB_H__
#define __RAINBOW_LINKLIB_H__

// -------------------------------------------------------------------------
#include "linklib_def.h"

#define LINKLIB_NAME	"rainbow"
#define LINKLIB			LINKLIB_NAME LINKLIB_SUFFIX
// -------------------------------------------------------------------------

#if !defined(__Linked_base_net_rainbow)
#	define __Linked_base_net_rainbow
#	pragma message(LINKLIB_MSG LINKLIB)
#	pragma linklib(LINKLIB)
#endif

#undef LINKLIB_NAME
#undef LINKLIB

// -------------------------------------------------------------------------

#endif /* __RAINBOW_LINKLIB_H__ */

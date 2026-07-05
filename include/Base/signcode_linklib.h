/* -------------------------------------------------------------------------
//	文件名		：	kgc/signcode_linklib.h
//	创建者		：	lailigao
//	创建时间	：	2004-10-11 0:11:03
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __KGC_SIGNCODE_LINKLIB_H__
#define __KGC_SIGNCODE_LINKLIB_H__

// -------------------------------------------------------------------------
#include "linklib_def.h"

#define	LINKLIB_NAME	"signcodescan"
#define	LINKLIB			LINKLIB_NAME LINKLIB_SUFFIX
// -------------------------------------------------------------------------

#if !defined(__LINKED_FILTERTEXTLIB)
#	define __LINKED_FILTERTEXTLIB
#	pragma message(LINKLIB_MSG LINKLIB)
#	pragma linklib(LINKLIB)
#endif

#undef	LINKLIB_NAME
#undef LINKLIB
// -------------------------------------------------------------------------


#endif /* __KGC_SIGNCODE_LINKLIB_H__ */

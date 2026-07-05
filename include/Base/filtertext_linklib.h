/* -------------------------------------------------------------------------
//	文件名		：	filtertext_linklib.h
//	创建者		：	llg
//	创建时间	：	2007-12-19 9:33:59
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __FILTERTEXT_LINKLIB_H__
#define __FILTERTEXT_LINKLIB_H__

// FilterText指定使用纯lib版本
#ifndef KD_USE_PURELIB
	#define KD_USE_PURELIB
#endif // #ifndef KD_USE_PURELIB

// -------------------------------------------------------------------------
#include "linklib_def.h"

#define	LINKLIB_NAME	"filtertext"
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

#endif /* __FILTERTEXT_LINKLIB_H__ */

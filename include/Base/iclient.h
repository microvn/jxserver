// -------------------------------------------------------------------------
//	文件名		：	IClient.h
//	创建者		：	lailigao
//	创建时间	：	2004-8-18 4:17:11
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __INCLUDE_ENTER_ICLIENT_H__
#define __INCLUDE_ENTER_ICLIENT_H__

#ifdef WIN32
	#include "net/win32/iclient.h"
#elif __linux
	#include "net/linux/iclient.h"
#endif

#endif // __INCLUDE_ENTER_ICLIENT_H__

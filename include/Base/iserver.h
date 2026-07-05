// -------------------------------------------------------------------------
//	文件名		：	IServer.h
//	创建者		：	lailigao
//	创建时间	：	2004-8-18 4:16:44
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __INCLUDE_ENTER_ISERVER_H__
#define __INCLUDE_ENTER_ISERVER_H__

#ifdef WIN32
	#include "net/win32/iserver.h"
#elif __linux
	#include "net/linux/iserver.h"
#endif

#endif // __INCLUDE_ENTER_ISERVER_H__

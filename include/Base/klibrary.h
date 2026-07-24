///////////////////////////////////////////////////////////////
//	
//  FileName		:   KLibrary.h
//  Composer		:	wangbin
//	latest Modify	:	2006-01-12
//  Comment			:   动态库的跨平台封装
//	
///////////////////////////////////////////////////////////////

#ifndef __KLIBRARY_H__
#define __KLIBRARY_H__

#ifdef __linux
#include "./klibrary_linux.h"
#else
#include "./klibrary_win32.h"
#endif

#endif //__KLIBRARY_H__

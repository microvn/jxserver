#ifndef KG_STDAFX_H
#define	KG_STDAFX_H

#ifdef WIN32

#define	_CRT_SECURE_NO_DEPRECATE
#define	_CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#define	_USE_32BIT_TIME_T
#define DECLSPEC_DEPRECATED
// 下面这个宏在2005下似乎会导致内存分配与释放不一致，大家就忍忍_malloca重定义的警告吧
//#define	_CRTDBG_MAP_ALLOC	// 使用CrtMemoryDebug 

#ifndef ASSERT
//临时解决ASSERT后宕机的问题，不能合并到Base库	by FanZai
//#include <assert.h>
//#define ASSERT(x)	assert(x)
#define ASSERT(x)	_ASSERT(x)
#endif

#else

#endif
#endif


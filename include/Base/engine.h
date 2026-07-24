//---------------------------------------------------------------------------
//  Engine头文件,引用Engine的其他模块包含此头问件来获得Engine各种接口数据的定义
//	Copyright : Kingsoft Season 2004
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2004-3-21
//---------------------------------------------------------------------------
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "kgstdafx.h"
#include "./commondefine.h"
#include "./engine/enginebase.h"

#include "./engine/debug.h"
#include "./engine/edonetimepad.h"		//译码与解码
#include "./engine/ksg_md5_string.h"	//md5译码与解码
#include "./engine/ksg_stringprocess.h"	//字符串处理
#include "./engine/text.h"				//文字处理
#include "./engine/file.h"				//文件操作
#include "./engine/filetype.h"			//ini文件类型
#include "./engine/kfilecache.h"		//文件缓存
#include "./engine/random.h"
#include "./engine/kcodepoints.h"
#include "./engine/klist.h"
#include "./engine/linkstructex.h"
#include "./engine/kbinstree.h"
#include "./engine/smartptr.h"
#include "./engine/kpolygon.h"
#include "./engine/klinkarray.h"		//旧功能代码，新代码请应用KOccupyList
#include "./engine/koccupylist.h"		//占用表
#include "./engine/klogfile.h"			//日志记录功能模块(Jizheng 2004-9-2)
#include "./engine/crc32.h"
#include "./engine/mutex.h"
#include "./engine/recyclebin.h"
#include "./engine/timer.h"
#include "./engine/kthread.h"
#include "./engine/objcache.h"
#include "./engine/kstrbase.h"
#include "./engine/kfile.h"
#include "./engine/kinifile.h"
#include "./engine/ktabfile.h"
#include "./engine/ktabfilectrl.h"
#include "./engine/kcolor.h"
#include "./engine/kobjallocator.h"
#include "./engine/kpackfilepartner.h"

#if defined(KD_SCRIPT_JX)
	// 全面封装的脚本系统	by FanZai
	#include "./engine/kjxscript.h"
	#include "./engine/kjxscriptset.h"
	#include "./engine/kscriptgroup.h"
	#include "./engine/luna.h"
#else	//KD_SCRIPT_JX
	#if defined(LUA_V_5)
		#include "./engine/kluascriptex.h"
	#else
		#include "./engine/kluascript.h"
	#endif	//LUA_V_5
#endif	//KD_SCRIPT_JX

//----以下接口的定义涉及到平台相关----
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <pthread.h>
	#include <assert.h>
#endif


#ifdef WIN32
	#include "./engine/kwin32app.h"
	#include "./engine/kurl.h"
	#include "./engine/kime.h"
#endif

#endif //ifndef _ENGINE_H_

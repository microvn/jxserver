//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2004 by Kingsoft
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef _ENGINE_RANDOM_H_
#define _ENGINE_RANDOM_H_

C_ENGINE_API unsigned int	g_Random(unsigned int nMax);		// 生成0 ~ nMax-1之间的一个随机数
C_ENGINE_API void			g_RandomSeed(unsigned int nSeed);	// 设置随机种子
C_ENGINE_API unsigned int	g_GetRandomSeed();					// 获得当前随机种子

#endif	//_ENGINE_RANDOM_H_


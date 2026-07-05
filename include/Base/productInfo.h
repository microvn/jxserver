/* -------------------------------------------------------------------------
//	文件名		：	productInfo.h
//	创建者		：	lailigao
//	创建时间	：	2004-7-1 14:54:02
//	功能描述	：	
//
// -----------------------------------------------------------------------*/
#ifndef __PRODUCTINFO_H__
#define __PRODUCTINFO_H__

// -------------------------------------------------------------------------
// PI - ProductInfo
// S - string
// N - number
#ifdef C_C_VERSION
	#define PIS_COMPANYNAME		"金山软件股份有限公司\0"
	#define PIS_COPYRIGHT		"版权所有 (C) 1995-2004 金山软件股份有限公司\0"
	#define PIS_PRODUCTNAME		"剑侠世界\0"
	#define PIS_PRODUCTVERSION	"1.00.00.2007\0"
	#define PIN_PRODUCTVERSION	1,00,00,2007
	#define PIS_ABOUTCOPYRIGHT	"版权所有 (C) 1995-2007 金山软件"
#else
	#define PIS_COMPANYNAME		"Kingsoft Corp.\0"
	#define PIS_COPYRIGHT		"Copyright (C) 1995-2007 Kingsoft Corp.\0"
	#define PIS_PRODUCTNAME		"World Of Swordman\0"
	#define PIS_PRODUCTVERSION	"1.00.00.2007\0"
	#define PIN_PRODUCTVERSION	1,00,00,2007
	#define PIS_ABOUTCOPYRIGHT	"Copyright (C) 1995-2007 Kingsoft"
#endif //#ifndef TRADITIONAL_CHINESE
// -------------------------------------------------------------------------

#endif /* __PRODUCTINFO_H__ */

// -------------------------------------------------------------------------
//	文件名		：	inoutmac.h
//	创建者		：	谢茂培 (Hsie)
//	创建时间	：	2003-07-20 14:47:18
//	功能描述	：	
//
// -------------------------------------------------------------------------
#ifndef __INOUTMAC_H__
#define __INOUTMAC_H__
#include "enginebase.h"

#ifndef KD_MAX_MACADDRESS_LEN
	#define KD_MAX_MACADDRESS_LEN      8
#endif // KD_MAX_MACADDRESS_LEN

//此函数会确保两个地址均有
//返回值为网卡数量
ENGINE_API int gGetMacAndIPAddress(
    unsigned char *pMacAddressIn,   unsigned long *pIPAddressIn, 
    unsigned char  *pMacAddressOut, unsigned long *pIPAddressOut, 
    unsigned uMask = 0x0000a8c0, 
    int nMacLength = 6
);

//获取内外网IP列表
//返回值为网卡数量
struct KADAPTER_MACADDRESS
{
	CHAR szMacAddress[KD_MAX_MACADDRESS_LEN];
};

ENGINE_API int gGetIPAddressList(
					KADAPTER_MACADDRESS* pMacAddressIns, DWORD* pdwIPAddressIns, DWORD& dwInIPCount, 
					KADAPTER_MACADDRESS* pMacAddressOuts, DWORD* pdwIPAddressOuts, DWORD& dwOutIPCount, 
					DWORD dwListCount, DWORD dwMask = 0x0000a8c0);

#endif // __INOUTMAC_H__

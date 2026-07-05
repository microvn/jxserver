/* -------------------------------------------------------------------------
//	文件名		：	kguid.h
//	创建者		：	wangbin
//	创建时间	：	2008-3-19 21:00:00
//	功能描述	：	GUID类
//
// -----------------------------------------------------------------------*/
#ifndef __KGUID_H__
#define __KGUID_H__

#include "ksdef.h"

class KGuidString
{
public:
	KGuidString(const GUID* pGuid = NULL)
	{
		if (pGuid)
			MakeString(pGuid);
		else
			m_szGuid[0] = '\0';
	}
	VOID Clear()
	{
		m_szGuid[0] = '\0';
	}
	LPCSTR GetString() const
	{
		return m_szGuid;
	}
	LPCSTR MakeString(const GUID* pGuid)
	{
		INT nIndex = 0;
		unsigned char* pData = (unsigned char*)pGuid;
		m_szGuid[nIndex++] = '{';
		for (INT i = 0; i < sizeof(GUID); i++)
		{
			unsigned char ch[2];
			ch[0] = (pData[i] >> 4) & 0xf;
			ch[1] = pData[i] & 0xf;
			for (INT j = 0; j < 2; j++)
			{
				UCHAR c = ch[j];
				if (c < 10)
					m_szGuid[nIndex++] = '0' + c;
				else
					m_szGuid[nIndex++] = 'A' + c - 10;
			}
		}
		m_szGuid[nIndex++] = '}';
		m_szGuid[nIndex] = '\0';
		return m_szGuid;
	}
private:
	char m_szGuid[sizeof(GUID) * 2 + 3];
};

class KGuid
{
public:
	KGuid(CONST GUID* pGuid = NULL)
	{
		if (pGuid)
		{
			SetGuid(pGuid);
		}
		else
		{
			memset(&m_sGuid, 0, sizeof(m_sGuid));
		}
	}
	inline VOID SetGuid(CONST GUID *pGuid)
	{
		m_sGuid = *pGuid;
		m_cString.MakeString(pGuid);
	}
	inline CONST GUID& GetGuid() CONST
	{
		return m_sGuid;
	}
	inline LPCSTR GetString() CONST
	{
		return m_cString.GetString();
	}
	inline VOID Clear()
	{
		memset(&m_sGuid, 0, sizeof(m_sGuid));
		m_cString.Clear();
	}
private:
	GUID			m_sGuid;
	KGuidString		m_cString;
};
#endif //__KGUID_H__

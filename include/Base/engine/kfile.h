//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KFile.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KFile_H
#define KFile_H

#include "engine.h"


class KFile
{
private:
	IFile*	m_pNativeFile;

public:
	KFile()
	{
		m_pNativeFile = NULL;
	}

	~KFile()
	{
		SAFE_RELEASE(m_pNativeFile);
	}

public:
	BOOL	Open(LPCSTR szFileName)
	{
		SAFE_RELEASE(m_pNativeFile);

		m_pNativeFile = g_OpenFile(szFileName);

		if (!m_pNativeFile)
			return FALSE;

		return TRUE;
	}
	BOOL	Create(LPCSTR szFileName)
	{
		SAFE_RELEASE(m_pNativeFile);

		m_pNativeFile = g_CreateFile(szFileName);

		if (!m_pNativeFile)
			return FALSE;

		return TRUE;
	}
	BOOL	Append(LPCSTR szFileName)
	{
		SAFE_RELEASE(m_pNativeFile);

		m_pNativeFile = g_OpenFile(szFileName, false, true);

		if (!m_pNativeFile)
			return FALSE;

		return TRUE;
	}

	void	Close()
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return;
		}

		m_pNativeFile->Close();
	}

	DWORD	Read(LPVOID lpBuffer, DWORD dwReadBytes)
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->Read(lpBuffer, dwReadBytes);
	}
	DWORD	Write(LPVOID lpBuffer, DWORD dwWriteBytes)
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->Write(lpBuffer, dwWriteBytes);
	}

	void*	GetBuffer()
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->GetBuffer();
	}

	DWORD	Seek(LONG lDistance, DWORD dwMoveMethod)
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->Seek(lDistance, dwMoveMethod);
	}
	DWORD	Tell()
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->Tell();
	}

	DWORD	Size()
	{
		if (!m_pNativeFile)
		{
			_ASSERT(FALSE);
			return 0;
		}

		return m_pNativeFile->Size();
	}
};

//---------------------------------------------------------------------------
#endif

//*********************************************************************
// file		: KInstance.h
// author	: wangbin
// datetime	: 2004-09-22 15:27
// comment	: 进程唯一实例
//*********************************************************************
#ifndef __KINSTANCE_H__
#define __KINSTANCE_H__

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef WIN32
class KInstance
{
public:
	// 检查是否存在某个名字对应的实例
	static BOOL IsRunning(LPCSTR pszName)
	{
		char szName[MAX_PATH];
		_snprintf(szName, MAX_PATH - 1, "Global\\%s", pszName);
		szName[MAX_PATH - 1] = 0;

		HANDLE handle = ::OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, szName);
		if (handle)
			::CloseHandle(handle);
		return (handle != NULL);
	}

	KInstance(LPCSTR pszName) : m_hLock(NULL)
	{
		ASSERT(pszName);
		strncpy(m_szName, pszName, sizeof(m_szName));
		m_szName[sizeof(m_szName) - 1] = '\0';
	}
	~KInstance()
	{
		if (m_hLock)
		{
			::CloseHandle(m_hLock);
			m_hLock = NULL;
		}
	}
	BOOL Lock(BOOL bShare = FALSE)
	{
		if (m_hLock)
		{
			_ASSERT(FALSE);
			return FALSE;
		}

		char szName[MAX_PATH];
		_snprintf(szName, MAX_PATH - 1, "Global\\%s", m_szName);
		szName[MAX_PATH - 1] = 0;
		HANDLE hLock = ::OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, szName);
		if (hLock)
		{
			if (bShare)
			{
				m_hLock = hLock;
				return TRUE;
			}
			else
			{
				::CloseHandle(hLock);
				return FALSE;
			}
		}
		m_hLock = ::CreateSemaphoreA(NULL, 1, 1, szName);
		return m_hLock != NULL;
	}
	VOID Unlock()
	{
		if (m_hLock)
		{
			::CloseHandle(m_hLock);
			m_hLock = NULL;
		}
	}
private:
	HANDLE	m_hLock;
	char	m_szName[128];
};

#else
class KInstance
{
public:
	KInstance(LPCSTR pszName) : m_nShmId(-1), m_pData(NULL)
	{
		OpenShareMemory(pszName, TRUE, m_nShmId);
	}
	~KInstance()
	{
		if (m_pData)
		{
			::shmdt(m_pData);
			m_pData = NULL;
		}
		m_nShmId = -1;
	}
	// 检查是否存在某个名字对应的实例
	static BOOL IsRunning(LPCSTR pszName)
	{
		int nId = -1;
		if (!OpenShareMemory(pszName, FALSE, nId) || nId < 0)
			return FALSE;
		shmid_ds sInf;
		if (::shmctl(nId, IPC_STAT, &sInf) < 0)
			return FALSE;
		return sInf.shm_nattch > 0;
	}
	BOOL Lock(BOOL bShare = FALSE)
	{
		if (m_nShmId < 0)
			return FALSE;
		if (m_pData)
			return TRUE;
		shmid_ds sInf;
		if (::shmctl(m_nShmId, IPC_STAT, &sInf) < 0)
			return FALSE;
		if (sInf.shm_nattch > 0 && !bShare)
			return FALSE;
		m_pData = ::shmat(m_nShmId, NULL, 0);
		return m_pData != NULL;
	}
	void Unlock()
	{
		if (m_pData != NULL && m_nShmId >= 0)
		{
			::shmdt(m_pData);
			m_pData = NULL;
		}
	}
private:
	static BOOL OpenShareMemory(LPCSTR pszName, BOOL bCreate, int& nId)
	{
		char szFile[1024];
		::snprintf(szFile, sizeof(szFile) - 1, "/tmp/%s.key", pszName);
		szFile[sizeof(szFile) - 1] = 0;
		nId = -1;
		if (!CreateFile(szFile))
			return FALSE;
		key_t nKey = ::ftok(szFile, ::getpid());
		if (nKey == -1)
			return FALSE;
		int nMode = SHM_R | SHM_W | SHM_R >> 3 | SHM_R >> 6;
		int nShmId = ::shmget(nKey, 1, nMode);
		if (nShmId < 0 && errno == ENOENT && bCreate)
			nShmId = ::shmget(nKey, 1, nMode | IPC_CREAT | IPC_EXCL);
		if (nShmId < 0)
			return FALSE;
		nId = nShmId;
		return TRUE;
	}
	static BOOL CreateFile(LPCSTR pszFile)
	{
		if (::access(pszFile, F_OK) == 0)
			return TRUE;
		int fd = ::open(pszFile, O_CREAT);
		if (fd == -1)
			return FALSE;
		::close(fd);
		return TRUE;

	}
private:
	int		m_nShmId;
	void*	m_pData;
};
#endif

#endif //__KINSTANCE_H__

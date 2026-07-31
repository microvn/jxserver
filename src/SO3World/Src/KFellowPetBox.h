#ifndef _KFELLOWPETBOX_H_
#define _KFELLOWPETBOX_H_

#include <map>
#include "KCustomData.h"

class KPlayer;

struct KPetTimeLimitInfo
{
    int     eExistType;
    time_t  nGenTime;
    time_t  nMaxExistTime;
};

class KFellowPetBox
{
public:
    KFellowPetBox() : m_pPlayer(NULL) {}

    BOOL Init(KPlayer* pPlayer)
    {
        m_pPlayer = pPlayer;
        m_PetTimeLimitInfoMap.clear();
        memset(&m_PetData, 0, sizeof(m_PetData));
        return true;
    }

    void UnInit()
    {
        m_PetTimeLimitInfoMap.clear();
        memset(&m_PetData, 0, sizeof(m_PetData));
        m_pPlayer = NULL;
    }

    BOOL Load(BYTE* pbyData, size_t uDataLen, DWORD dwVersion)
    {
        BOOL bResult = false;
        BYTE* pbyOffset = pbyData;
        size_t uLeftSize = uDataLen;
        DWORD dwDataLen = 0;
        WORD wCount = 0;
        WORD wPetIndex = 0;
        KPetTimeLimitInfo Info;

        KGLOG_PROCESS_ERROR(pbyData);
        m_PetTimeLimitInfoMap.clear();
        memset(&m_PetData, 0, sizeof(m_PetData));

        if (dwVersion == 0)
        {
            KGLOG_PROCESS_ERROR(m_PetData.Load(pbyData, uDataLen));
            bResult = true;
            goto Exit0;
        }

        KGLOG_PROCESS_ERROR(dwVersion == 1);
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD) + sizeof(WORD));
        dwDataLen = *(DWORD*)pbyOffset;
        wCount = *(WORD*)(pbyOffset + sizeof(DWORD));
        pbyOffset += sizeof(DWORD) + sizeof(WORD);
        uLeftSize -= sizeof(DWORD) + sizeof(WORD);
        KGLOG_PROCESS_ERROR((size_t)dwDataLen <= uLeftSize);
        KGLOG_PROCESS_ERROR(m_PetData.Load(pbyOffset, dwDataLen));
        pbyOffset += dwDataLen;
        uLeftSize -= dwDataLen;

        while (wCount-- > 0)
        {
            KGLOG_PROCESS_ERROR(uLeftSize >= 11);
            wPetIndex = *(WORD*)pbyOffset;
            Info.eExistType = (int)*(BYTE*)(pbyOffset + 2);
            memcpy(&Info.nGenTime, pbyOffset + 3, sizeof(time_t));
            memcpy(&Info.nMaxExistTime, pbyOffset + 7, sizeof(time_t));
            m_PetTimeLimitInfoMap[(int)wPetIndex] = Info;
            pbyOffset += 11;
            uLeftSize -= 11;
        }

        bResult = true;
Exit0:
        if (!bResult)
        {
            m_PetTimeLimitInfoMap.clear();
            memset(&m_PetData, 0, sizeof(m_PetData));
        }
        return bResult;
    }

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
    {
        BOOL bResult = false;
        BYTE* pbyOffset = pbyBuffer;
        size_t uLeftSize = uBufferSize;
        size_t uDataLen = 0;
        std::map<int, KPetTimeLimitInfo>::const_iterator it;

        KGLOG_PROCESS_ERROR(puUsedSize);
        KGLOG_PROCESS_ERROR(pbyBuffer);
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD) + sizeof(WORD));
        pbyOffset += sizeof(DWORD) + sizeof(WORD);
        uLeftSize -= sizeof(DWORD) + sizeof(WORD);
        KGLOG_PROCESS_ERROR(m_PetData.Save(&uDataLen, pbyOffset, uLeftSize));
        KGLOG_PROCESS_ERROR(uDataLen <= 0xffff);
        *(DWORD*)pbyBuffer = (DWORD)uDataLen;
        pbyOffset += uDataLen;
        uLeftSize -= uDataLen;
        *(WORD*)(pbyBuffer + sizeof(DWORD)) = (WORD)m_PetTimeLimitInfoMap.size();
        KGLOG_PROCESS_ERROR(m_PetTimeLimitInfoMap.size() <= 0xffff);

        for (it = m_PetTimeLimitInfoMap.begin(); it != m_PetTimeLimitInfoMap.end(); ++it)
        {
            KGLOG_PROCESS_ERROR(uLeftSize >= 11);
            *(WORD*)pbyOffset = (WORD)it->first;
            *(BYTE*)(pbyOffset + 2) = (BYTE)it->second.eExistType;
            memcpy(pbyOffset + 3, &it->second.nGenTime, sizeof(time_t));
            memcpy(pbyOffset + 7, &it->second.nMaxExistTime, sizeof(time_t));
            pbyOffset += 11;
            uLeftSize -= 11;
        }

        *puUsedSize = uBufferSize - uLeftSize;
        bResult = true;
Exit0:
        return bResult;
    }

private:
    KPlayer* m_pPlayer;
    KCustomData<9> m_PetData;
    std::map<int, KPetTimeLimitInfo> m_PetTimeLimitInfoMap;
};

#endif

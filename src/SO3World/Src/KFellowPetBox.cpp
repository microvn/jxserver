#include "stdafx.h"
#include "KFellowPetBox.h"
#include "KPlayer.h"
#include "SO3World/KSO3World.h"

KFellowPetBox::KFellowPetBox()
{
    m_pPlayer = NULL;
}

KFellowPetBox::~KFellowPetBox()
{
    UnInit();
}

BOOL KFellowPetBox::Init(KPlayer* pPlayer)
{
    KGLOG_PROCESS_ERROR(pPlayer);
    m_pPlayer = pPlayer;
    return true;
Exit0:
    return false;
}

void KFellowPetBox::UnInit()
{
    m_pPlayer = NULL;
    m_PetData = KCustomData<9>();
    m_PetTimeLimitInfoMap.clear();
}

BOOL KFellowPetBox::IsOverdue(const KPetTimeLimitInfo& Info, time_t nTimeNow)
{
    switch (Info.eExistType)
    {
    case ketPermanent:
        return false;
    case ketOnLine:
    case ketOnlineAndOffLine:
        return Info.nGenTime + Info.nMaxExistTime <= nTimeNow;
    case ketOffLine:
    case ketTimeStamp:
        return Info.nMaxExistTime <= nTimeNow;
    default:
        return false;
    }
}

BOOL KFellowPetBox::IsAcquired(int nPetIndex)
{
    BOOL bAcquired = false;
    if (nPetIndex <= 0 || nPetIndex > 64)
        return false;
    return m_PetData.GetBit(nPetIndex - 1, &bAcquired) && bAcquired;
}

BOOL KFellowPetBox::GetTimeLimitInfo(int nPetIndex, int* pnExistType, time_t* pnGenTime, time_t* pnMaxExistTime)
{
    KPET_TIME_LIMIT_INFO_MAP::const_iterator it;
    if (!pnExistType || !pnGenTime || !pnMaxExistTime || !IsAcquired(nPetIndex))
        return false;
    it = m_PetTimeLimitInfoMap.find(nPetIndex);
    if (it == m_PetTimeLimitInfoMap.end())
        return false;
    *pnExistType = it->second.eExistType;
    *pnGenTime = it->second.nGenTime;
    *pnMaxExistTime = it->second.nMaxExistTime;
    return true;
}

BOOL KFellowPetBox::LoadTimeLimitInfo(const KPET_TIME_LIMIT_INFO_DB* pInfo, time_t nTimeNow, time_t nWorldTime)
{
    KPetTimeLimitInfo Info;
    if (!pInfo)
        return false;
    Info.eExistType = (KITEM_EXIST_TYPE)pInfo->byExistType;
    switch (Info.eExistType)
    {
    case ketOnLine:
        Info.nGenTime = pInfo->nGenTime + nWorldTime;
        Info.nMaxExistTime = pInfo->nMaxExistTime;
        break;
    case ketOffLine:
        Info.nGenTime = pInfo->nGenTime;
        Info.nMaxExistTime = pInfo->nMaxExistTime;
        if (Info.nMaxExistTime < nWorldTime)
        {
            m_PetData.SetBit(pInfo->wPetIndex, false);
            return true;
        }
        break;
    case ketOnlineAndOffLine:
    case ketTimeStamp:
        Info.nGenTime = pInfo->nGenTime;
        Info.nMaxExistTime = pInfo->nMaxExistTime;
        break;
    default:
        return false;
    }
    if (IsOverdue(Info, nTimeNow))
    {
        m_PetData.SetBit(pInfo->wPetIndex, false);
        return true;
    }
    return m_PetTimeLimitInfoMap.insert(std::make_pair((int)pInfo->wPetIndex, Info)).second;
}

BOOL KFellowPetBox::Load_V0(BYTE* pbyData, size_t uDataLen)
{
    return m_PetData.Load(pbyData, uDataLen);
}

BOOL KFellowPetBox::Load_V1(BYTE* pbyData, size_t uDataLen)
{
    DWORD dwPetDataSize = 0;
    WORD wCount = 0;
    BYTE* pbyOffset = pbyData;
    size_t i = 0;
    time_t nTimeNow = g_pSO3World->m_nCurrentTime;
    time_t nWorldTime = nTimeNow - m_pPlayer->m_nLastSaveTime;
    if (!pbyData || uDataLen <= 5)
        return false;
    memcpy(&dwPetDataSize, pbyOffset, sizeof(dwPetDataSize));
    memcpy(&wCount, pbyOffset + sizeof(dwPetDataSize), sizeof(wCount));
    pbyOffset += 6;
    uDataLen -= 6;
    if (dwPetDataSize + (size_t)wCount * sizeof(KPET_TIME_LIMIT_INFO_DB) > uDataLen ||
        !m_PetData.Load(pbyOffset, dwPetDataSize))
        return false;
    pbyOffset += dwPetDataSize;
    for (i = 0; i < wCount; ++i, pbyOffset += sizeof(KPET_TIME_LIMIT_INFO_DB))
        if (!LoadTimeLimitInfo((const KPET_TIME_LIMIT_INFO_DB*)pbyOffset, nTimeNow, nWorldTime))
            return false;
    return true;
}

BOOL KFellowPetBox::Load(BYTE* pbyData, size_t uDataLen, DWORD dwVersion)
{
    BOOL bResult = false;
    if (dwVersion == 0)
        bResult = Load_V0(pbyData, uDataLen);
    else if (dwVersion == 1)
        bResult = Load_V1(pbyData, uDataLen);
    if (!bResult)
    {
        m_PetTimeLimitInfoMap.clear();
        m_PetData = KCustomData<9>();
    }
    return bResult;
}

BOOL KFellowPetBox::Acquire(int nPetIndex)
{
    return !IsAcquired(nPetIndex) && m_PetData.SetBit(nPetIndex - 1, true);
}

BOOL KFellowPetBox::Remove(int nPetIndex)
{
    if (!m_PetData.SetBit(nPetIndex - 1, false))
        return false;
    m_PetTimeLimitInfoMap.erase(nPetIndex);
    return true;
}

void KFellowPetBox::Activate()
{
    KPET_TIME_LIMIT_INFO_MAP::iterator it = m_PetTimeLimitInfoMap.begin();
    time_t nTimeNow = time(NULL);
    while (it != m_PetTimeLimitInfoMap.end())
    {
        if (IsOverdue(it->second, nTimeNow))
        {
            m_PetData.SetBit(it->first - 1, false);
            m_PetTimeLimitInfoMap.erase(it++);
        }
        else
            ++it;
    }
}

BOOL KFellowPetBox::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL    bResult = false;
    BOOL    bRetCode = false;
    size_t  uUsedSize = 0;
    size_t  uLeftSize = uBufferSize;
    BYTE*   pbyOffset = pbyBuffer;
    KPET_TIME_LIMIT_INFO_MAP::const_iterator it;

    KGLOG_PROCESS_ERROR(puUsedSize);
    KGLOG_PROCESS_ERROR(pbyBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize > 5);

    pbyOffset += 6;
    uLeftSize -= 6;

    bRetCode = m_PetData.Save(&uUsedSize, pbyOffset, uLeftSize);
    KGLOG_PROCESS_ERROR(bRetCode);
    *(DWORD*)pbyBuffer = (DWORD)uUsedSize;
    *(WORD*)(pbyBuffer + 4) = (WORD)m_PetTimeLimitInfoMap.size();

    pbyOffset += uUsedSize;
    uLeftSize -= uUsedSize;
    KGLOG_PROCESS_ERROR(uLeftSize >= 11 * m_PetTimeLimitInfoMap.size());

    for (it = m_PetTimeLimitInfoMap.begin(); it != m_PetTimeLimitInfoMap.end(); ++it)
    {
        *(WORD*)pbyOffset = (WORD)it->first;
        pbyOffset[2] = (BYTE)it->second.eExistType;
        *(DWORD*)(pbyOffset + 3) = (DWORD)it->second.nGenTime;
        *(DWORD*)(pbyOffset + 7) = (DWORD)it->second.nMaxExistTime;
        pbyOffset += 11;
        uLeftSize -= 11;
    }

    *puUsedSize = uBufferSize - uLeftSize;
    bResult = true;
Exit0:
    return bResult;
}

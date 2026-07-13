#include "stdafx.h"
#include "KCurrencyList.h"
#include "KPlayer.h"

// v2.5 NEW (RE'd from v246). See KCurrencyList.h + docs/currency_port/.

// DB blob byte layout pinned from v246 Save 081e7eac / Load 081e7c1c (+ getter/setter machine
// code): [BYTE nCount=6][ KCURRENCY_INFO x nCount ]. Per entry (0x24=36B): nType/nValue/(hole)/
// nRemainSpace/nLastResetTime/reserved[16]. m_nMaxValue is config-derived, NOT persisted; Load
// takes type from the array index (the stored nType is written for reference but ignored on load).
#pragma pack(1)
struct KCURRENCY_DB_DATA
{
    BYTE byCount;
    struct KCURRENCY_INFO
    {
        int  nType;             // +0x00 (saved for reference; Load uses the index)
        int  nValue;            // +0x04
        int  nMaxValueHole;     // +0x08 (config-derived hole; kept for byte layout, zeroed)
        int  nRemainSpace;      // +0x0c
        int  nLastResetTime;    // +0x10
        BYTE reserved[16];      // +0x14
    } Info[0];
};
#pragma pack()

// Lock the entry byte size (v246 KCURRENCY_INFO = 0x24).
typedef char _CHK_CURRENCY_INFO[(sizeof(KCURRENCY_DB_DATA::KCURRENCY_INFO) == 0x24) ? 1 : -1];

KCurrencyList::KCurrencyList()
{
}

KCurrencyList::~KCurrencyList()
{
}

// v246 Init (081e80f8): Init each of the 6 currencies with its type index.
BOOL KCurrencyList::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;
    int  i       = 0;

    assert(pPlayer);

    m_pPlayer = pPlayer;

    for (i = 0; i < CURRENCY_TYPE_COUNT; i++)
    {
        m_CurrencyList[i].Init(pPlayer, i);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KCurrencyList::UnInit()
{
    int i = 0;

    for (i = 0; i < CURRENCY_TYPE_COUNT; i++)
    {
        m_CurrencyList[i].UnInit();
    }

    m_pPlayer = NULL;
}

// v246 KCurrencyList::Activate (081e8082): tick every currency's periodic reset.
void KCurrencyList::Activate()
{
    int i = 0;

    for (i = 0; i < CURRENCY_TYPE_COUNT; i++)
    {
        m_CurrencyList[i].Activate();
    }
}

// v246 GetCurrency (081e7e4a).
KCurrency* KCurrencyList::GetCurrency(int nType)
{
    KCurrency* pResult = NULL;

    KGLOG_PROCESS_ERROR(nType >= 0 && nType < CURRENCY_TYPE_COUNT);

    pResult = &m_CurrencyList[nType];
Exit0:
    return pResult;
}

// v246 Save (081e7eac): count byte + one KCURRENCY_INFO per currency.
BOOL KCurrencyList::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    int    i         = 0;

    assert(puUsedSize);
    assert(pbyBuffer);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE));

    *pbyOffset = (BYTE)CURRENCY_TYPE_COUNT;
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    for (i = 0; i < CURRENCY_TYPE_COUNT; i++)
    {
        KCURRENCY_DB_DATA::KCURRENCY_INFO* pInfo = NULL;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KCURRENCY_DB_DATA::KCURRENCY_INFO));

        pInfo = (KCURRENCY_DB_DATA::KCURRENCY_INFO*)pbyOffset;
        memset(pInfo, 0, sizeof(*pInfo));   // deterministic blob (v246 leaves the +8 hole garbage)

        pInfo->nType          = m_CurrencyList[i].GetType();
        pInfo->nValue         = m_CurrencyList[i].GetValue();
        pInfo->nRemainSpace   = m_CurrencyList[i].GetRemainSpace();
        pInfo->nLastResetTime = m_CurrencyList[i].GetLastResetTime();

        pbyOffset += sizeof(*pInfo);
        uLeftSize -= sizeof(*pInfo);
    }

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

// v246 Load (081e7c1c): count byte, then per entry restore value/remainSpace/lastResetTime;
// type = array index. maxValue stays as Init set it (config-derived).
BOOL KCurrencyList::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL   bResult   = false;
    BYTE*  pbyOffset = pbyData;
    size_t uLeftSize = uDataLen;
    int    nCount    = 0;
    int    i         = 0;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(BYTE));

    nCount = (int)*pbyOffset;
    pbyOffset += sizeof(BYTE);
    uLeftSize -= sizeof(BYTE);

    KGLOG_PROCESS_ERROR(nCount <= CURRENCY_TYPE_COUNT);
    KGLOG_PROCESS_ERROR(uLeftSize >= (size_t)nCount * sizeof(KCURRENCY_DB_DATA::KCURRENCY_INFO));

    for (i = 0; i < nCount; i++)
    {
        KCURRENCY_DB_DATA::KCURRENCY_INFO* pInfo = (KCURRENCY_DB_DATA::KCURRENCY_INFO*)pbyOffset;

        m_CurrencyList[i].SetType(i);
        m_CurrencyList[i].SetValue(pInfo->nValue);
        m_CurrencyList[i].SetRemainSpace(pInfo->nRemainSpace);
        m_CurrencyList[i].SetLastResetTime(pInfo->nLastResetTime);

        pbyOffset += sizeof(*pInfo);
        uLeftSize -= sizeof(*pInfo);
    }

    KGLOG_PROCESS_ERROR(uLeftSize == 0);

    bResult = true;
Exit0:
    return bResult;
}

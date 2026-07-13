#include "stdafx.h"
#include "KCurrency.h"
#include "KPlayer.h"

// v2.5 NEW (RE'd from v246). See KCurrency.h + docs/currency_port/.

// ---------------------------------------------------------------------------
// Per-type CONFIG (v246 reads 4 arrays + period/week-base from g_pSO3World early
// offsets, indexed by type). That config is a NEW v2.5 settings subsystem whose
// source table is NOT yet identified (docs/currency_port WORKLOG [RE-5]). These
// accessors are the SINGLE wire-point: dormant now (return 0) -> currencies hold
// nothing and Activate is a guarded no-op. The config-source slice replaces these
// bodies to read from settings, flipping every currency live with no other change.
// ---------------------------------------------------------------------------
static int    GetCurrencyMaxValue(int /*nType*/)      { return 0; }
static int    GetCurrencyRemainDefault(int /*nType*/) { return 0; }
static int    GetCurrencyCarryFlag(int /*nType*/)     { return 0; }
static int    GetCurrencyCarryPct(int /*nType*/)      { return 0; }
static int    GetCurrencyResetPeriod()                { return 0; }  // 0 => Activate no-op (also avoids div-by-0)
static time_t GetCurrencyWeekBase()                   { return 0; }

KCurrency::KCurrency()
{
}

KCurrency::~KCurrency()
{
}

// v246 KCurrency::Init(KPlayer*, int): value/lastReset 0; caps from config.
BOOL KCurrency::Init(KPlayer* pPlayer, int nType)
{
    BOOL bResult = false;

    assert(pPlayer);
    KGLOG_PROCESS_ERROR(nType >= 0 && nType < CURRENCY_TYPE_COUNT);

    m_pPlayer        = pPlayer;
    m_nType          = nType;
    m_nValue         = 0;
    m_nLastResetTime = 0;
    m_nMaxValue      = GetCurrencyMaxValue(nType);
    m_nRemainSpace   = GetCurrencyRemainDefault(nType);

    bResult = true;
Exit0:
    return bResult;
}

void KCurrency::UnInit()
{
    m_pPlayer        = NULL;
    m_nValue         = 0;
    m_nRemainSpace   = 0;
    m_nLastResetTime = 0;
}

// v246 AddCurrency (081e846c). Positive = earn: clamp to (maxValue-value) then to remainSpace,
// decrement remainSpace. Negative = spend: bypasses caps, value clamps >= 0.
BOOL KCurrency::AddCurrency(int nAddValue)
{
    BOOL bResult = false;

    if (nAddValue != 0)
    {
        if (nAddValue > 0)
        {
            if (m_nValue >= m_nMaxValue)
                goto Exit1;                              // already at total cap -> no earn

            KGLOG_PROCESS_ERROR(m_nValue < (m_nValue + nAddValue));   // overflow guard

            if (m_nMaxValue < (m_nValue + nAddValue))
                nAddValue = m_nMaxValue - m_nValue;      // clamp to total cap
        }

        if (m_nRemainSpace < nAddValue)                  // clamp to periodic earn cap (only bites when add>0)
            nAddValue = m_nRemainSpace;

        if (nAddValue > 0)
        {
            m_nRemainSpace -= nAddValue;
            // if (m_nRemainSpace == 0) -> v246 notifies "earn allowance exhausted" (NEW packet, deferred)
        }

        m_nValue += nAddValue;
        if (m_nValue < 0)
            m_nValue = 0;
    }

Exit1:
    // v246: log if |change| >= a per-player threshold, then DoSyncCurrency to client.
    // Both are NEW packets -> deferred with the currency-sync slice (docs/currency_port).
    bResult = true;
Exit0:
    return bResult;
}

// v246 AddRemainSpace (081e83c6): refill the earn allowance.
BOOL KCurrency::AddRemainSpace(int nAddRemainSpaceValue)
{
    BOOL bResult = false;

    if (nAddRemainSpaceValue != 0)
    {
        if (nAddRemainSpaceValue > 0)
            KGLOG_PROCESS_ERROR(m_nRemainSpace + nAddRemainSpaceValue >= 0);   // overflow guard

        m_nRemainSpace += nAddRemainSpaceValue;
        if (m_nRemainSpace < 0)
            m_nRemainSpace = 0;

        // v246: DoSyncCurrency (NEW packet, deferred)
    }

    bResult = true;
Exit0:
    return bResult;
}

// v246 Activate (081e861a): periodic (weekly) reset of the earn allowance, with carryover.
// type 4 is exempt. Guarded to a no-op while the reset period is unconfigured (dormant).
void KCurrency::Activate()
{
    int    nPeriod   = 0;
    time_t nBase     = 0;
    time_t nNow      = 0;
    int    nDefault  = 0;

    if (m_nType == 4)
        return;                                          // v246: type 4 never resets

    nPeriod = GetCurrencyResetPeriod();
    if (nPeriod <= 0)
        return;                                          // no config -> no reset (also avoids div-by-0)

    nBase = GetCurrencyWeekBase();
    nNow  = g_pSO3World->m_nCurrentTime;

    if (m_nLastResetTime == 0)
    {
        m_nLastResetTime = (int)nNow;
        m_nRemainSpace   = GetCurrencyRemainDefault(m_nType);
    }
    else
    {
        int nPrevIdx = (int)((m_nLastResetTime - nBase) / nPeriod);
        int nCurIdx  = (int)((nNow - nBase) / nPeriod);

        if (nCurIdx <= nPrevIdx)
            return;                                      // still the same period

        nDefault = GetCurrencyRemainDefault(m_nType);
        {
            int nCarryPct = GetCurrencyCarryFlag(m_nType) ? GetCurrencyCarryPct(m_nType) : 0;
            int nCarry    = ((m_nRemainSpace + nDefault * (nCurIdx - nPrevIdx - 1)) * nCarryPct) / 100;
            KGLOG_PROCESS_ERROR(nCarry >= 0);
            m_nRemainSpace   = nCarry + nDefault;
            m_nLastResetTime = (int)nNow;
        }
    }

    // v246: notify + DoSyncCurrency (NEW packets, deferred)
Exit0:
    return;
}

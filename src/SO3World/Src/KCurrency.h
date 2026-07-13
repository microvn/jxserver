#ifndef _KCURRENCY_H_
#define _KCURRENCY_H_

#include "Global.h"

// v2.5 NEW (RE'd from v246). One player-held currency of the capped/periodic-reset kind
// (限购货币): a value with a total cap (m_nMaxValue) and a per-period earn allowance
// (m_nRemainSpace) that resets on a timer. 6 types (index 0..5). See docs/currency_port.

#define CURRENCY_TYPE_COUNT  6   // v246 cbtTotal

class KPlayer;

class KCurrency
{
public:
    KCurrency();
    ~KCurrency();

    BOOL Init(KPlayer* pPlayer, int nType);
    void UnInit();

    int  GetType()                 { return m_nType; }
    void SetType(int nType)        { m_nType = nType; }
    int  GetValue()                { return m_nValue; }
    void SetValue(int nValue)      { m_nValue = nValue; }
    int  GetMaxValue()             { return m_nMaxValue; }
    int  GetRemainSpace()          { return m_nRemainSpace; }
    void SetRemainSpace(int nVal)  { m_nRemainSpace = nVal; }
    int  GetLastResetTime()        { return m_nLastResetTime; }
    void SetLastResetTime(int nVal){ m_nLastResetTime = nVal; }

    // Anti-hack: enforces BOTH the total cap and the periodic earn cap. MUST stay C++.
    // Positive = earn (capped); negative = spend (uncapped, clamps value >= 0).
    BOOL AddCurrency(int nAddValue);
    // Refill the periodic earn allowance.
    BOOL AddRemainSpace(int nAddRemainSpaceValue);
    // Periodic (weekly) reset of the earn allowance. No-op while config is dormant.
    void Activate();

private:
    KPlayer* m_pPlayer;         // @0x00
    int      m_nType;           // @0x04
    int      m_nValue;          // @0x08
    int      m_nMaxValue;       // @0x0c  (config-derived; NOT persisted)
    int      m_nRemainSpace;    // @0x10
    int      m_nLastResetTime;  // @0x14
};

#endif  //_KCURRENCY_H_

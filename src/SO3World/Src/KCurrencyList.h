#ifndef _KCURRENCY_LIST_H_
#define _KCURRENCY_LIST_H_

#include "Global.h"
#include "KCurrency.h"

// v2.5 NEW (RE'd from v246). Per-player container of the 6 capped/periodic currencies.
// Persisted via role-block rbtCurrencyData. See docs/currency_port/.

class KPlayer;

class KCurrencyList
{
public:
    KCurrencyList();
    ~KCurrencyList();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    void Activate();                       // periodic reset of every currency's earn allowance

    KCurrency* GetCurrency(int nType);

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

private:
    KPlayer*  m_pPlayer;
    KCurrency m_CurrencyList[CURRENCY_TYPE_COUNT];
};

#endif  //_KCURRENCY_LIST_H_

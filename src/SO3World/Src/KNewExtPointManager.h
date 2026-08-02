#ifndef _KNEW_EXT_POINT_MANAGER_H_
#define _KNEW_EXT_POINT_MANAGER_H_

#include <map>
#include "Global.h"
#include "Engine/KMemory.h"

class KPlayer;
struct KSyncGSNEPInfo;

struct KNewExtPointInfo
{
    int  nValue;
    BOOL bLocked;
};

class KNewExtPointManager
{
public:
    typedef std::map<
        int,
        KNewExtPointInfo,
        std::less<int>,
        KMemory::KAllocator<std::pair<int, KNewExtPointInfo> >
    > KNEW_EXT_POINT_MAP;

    KNewExtPointManager();
    ~KNewExtPointManager();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();
    BOOL AddNewExtPoint(int nKey, int nValue, BOOL bLocked);
    BOOL CanSetExtPoint(int nIndex);
    BOOL GetNewExtPoint(int nIndex, int* pnValue) const;
    BOOL GetNewExtPointByBits(int nIndex, int nBitIndex, int nBitLength, int* pnValue) const;
    BOOL SetNewExtPoint(int nIndex, int nChangeValue);
    BOOL SetExtPointByBits(int nIndex, int nBitIndex, int nBitLength, int nChangeValue);
    BOOL GetGSNewExtPoint(int nLastKey, KSyncGSNEPInfo* pInfo, int nMaxCount, int* pnCount, BOOL* pbSyncFinish) const;
    BOOL OnChangeNewExtPoint(int nIndex, int nOldValue, int nChangeValue, int nCurrentValue, int nActionCode);

private:
    KNEW_EXT_POINT_MAP m_NEPMap;
    KPlayer*                        m_pPlayer;
};

#endif

#ifndef _KNEW_EXT_POINT_MANAGER_H_
#define _KNEW_EXT_POINT_MANAGER_H_

#include <map>

class KPlayer;

struct KNewExtPointInfo
{
    int  nValue;
    BOOL bLocked;
};

class KNewExtPointManager
{
public:
    KNewExtPointManager() : m_pPlayer(NULL) {}

    BOOL Init(KPlayer* pPlayer)
    {
        m_pPlayer = pPlayer;
        return true;
    }

    void UnInit()
    {
        m_NEPMap.clear();
        m_pPlayer = NULL;
    }

    BOOL AddNewExtPoint(int nKey, int nValue, int bLocked)
    {
        KNewExtPointInfo info;
        info.nValue = nValue;
        info.bLocked = (BOOL)bLocked;
        return m_NEPMap.insert(std::make_pair(nKey, info)).second;
    }

    BOOL GetNewExtPoint(int nKey, int* pnValue) const
    {
        std::map<int, KNewExtPointInfo>::const_iterator it;

        if (!pnValue)
            return false;

        it = m_NEPMap.find(nKey);
        if (it == m_NEPMap.end())
            return false;

        *pnValue = it->second.nValue;
        return true;
    }

private:
    std::map<int, KNewExtPointInfo> m_NEPMap;
    KPlayer*                        m_pPlayer;
};

#endif

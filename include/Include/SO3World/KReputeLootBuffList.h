#ifndef _KREPUTE_LOOT_BUFF_LIST_H_
#define _KREPUTE_LOOT_BUFF_LIST_H_
#include <map>
typedef std::map<DWORD, DWORD> KREPUTE_LOOT_BUFF_LIST;
class KReputeLootBuffList
{
public:
    BOOL Init();
    BOOL UnInit();
    const KREPUTE_LOOT_BUFF_LIST& GetList() const;
private:
    KREPUTE_LOOT_BUFF_LIST m_ReputeLootBuffList;
};
#endif

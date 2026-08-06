#ifndef _KPENDENT_OLD_DATA_INFO_LIST_H_
#define _KPENDENT_OLD_DATA_INFO_LIST_H_

#include <map>

class KPendentOldDataInfoList
{
public:
    BOOL Init();
    BOOL UnInit();
    DWORD GetOldWaistPendentItemID(int nRepresentID) const;
    DWORD GetOldBackPendentItemID(int nRepresentID) const;
    DWORD GetOldFacePendentItemID(int nRepresentID) const;
    DWORD GetDuplicateItemID(DWORD dwRepresentID) const;

private:
    typedef std::map<int, DWORD> KREPRESENT_ITEM_MAP;
    typedef std::map<DWORD, DWORD> KDUPLICATE_ITEM_MAP;
    KREPRESENT_ITEM_MAP m_WaistRepresentID2ItemIDMap;
    KREPRESENT_ITEM_MAP m_BackRepresentID2ItemIDMap;
    KREPRESENT_ITEM_MAP m_FaceRepresentID2ItemIDMap;
    KDUPLICATE_ITEM_MAP m_DuplicateRepresetIDItemTransMap;
};

#endif

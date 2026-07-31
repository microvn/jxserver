#ifndef _KDIRECT_MENTOR_CACHE_H_
#define _KDIRECT_MENTOR_CACHE_H_

#include <map>
#include "KMentorDef.h"

struct KDirectMentorRecordCache
{
    DWORD dwTAEquipsScore;
    BYTE  byState;
};

class KDirectMentorCache
{
public:
    BOOL Init() { return true; }
    BOOL UnInit() { Clear(); return true; }
    BOOL AddMentorData(DWORD dwMentor, DWORD dwApprentice, const KDirectMentorRecordCache& record);
    BOOL UpdateMentorData(DWORD dwMentor, DWORD dwApprentice, const KDirectMentorRecordCache& record);
    BOOL DeleteMentorRecord(uint64_t uKey);
    void Clear();
    int PickupTAEquipsScore(DWORD dwMentorID);

private:
    typedef std::map<uint64_t, KDirectMentorRecordCache> RecordMap;
    typedef std::map<uint64_t, KDirectMentorRecordCache*> PointerMap;
    RecordMap  m_CacheMTable;
    PointerMap m_CacheATable;
};

#endif

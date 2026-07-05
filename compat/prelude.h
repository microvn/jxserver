/* Force-included first in every TU. KingSoft headers use std names (string,
 * vector, pair, map...) unqualified, relying on MSVC leaking std into global
 * scope. gcc doesn't, so we pull the common std headers and open the namespace
 * once, up front. */
#ifndef _JX3_COMPAT_PRELUDE_H
#define _JX3_COMPAT_PRELUDE_H

/* MSVC fixed-width keywords — defined FIRST so headers that use __int64 before
 * pulling windows.h still see it (KEntryList.h etc.). */
#ifndef __int64
#define __int64  long long
#endif
#ifndef __int32
#define __int32  int
#endif
#ifndef __int16
#define __int16  short
#endif
#ifndef __int8
#define __int8   char
#endif

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <utility>
#include <algorithm>
#include <functional>

using namespace std;

/* KingSoft base string types. The real engine defines these as pooled basic_string
 * over KMemory::KAllocator, but the reconstruction tree is std::string-based for
 * self-consistency (adopting the pooled form regressed the build). */
typedef std::string  kstring;
typedef std::wstring kwstring;

#endif

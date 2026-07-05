/* Linux compat <windows.h> for the JX3 server build.
 * Types (DWORD/BOOL/BYTE/WORD...) are provided portably by kstypes.h under
 * __linux, so this header deliberately does NOT redefine them — it only supplies
 * calling-convention macros, __declspec noop, and a few memory helpers.
 * Grows on demand as the compile loop surfaces missing Win32 surface. */
#ifndef _JX3_COMPAT_WINDOWS_H
#define _JX3_COMPAT_WINDOWS_H

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

/* MSVC fixed-width keywords (macros so `unsigned __int64` also works) */
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

/* calling conventions / attributes — no-ops on Linux gcc */
#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef WINAPIV
#define WINAPIV
#endif
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

/* NB: no min/max MACROS here — they clobber std::min/max and member fns named
 * min/max. compat/prelude.h does `using namespace std` + <algorithm>, so bare
 * max(a,b)/min(a,b) already resolve to the std function templates. */

/* memory helpers */
#ifndef ZeroMemory
#define ZeroMemory(d,l)   memset((d),0,(l))
#endif
#ifndef CopyMemory
#define CopyMemory(d,s,l) memcpy((d),(s),(l))
#endif
#ifndef MoveMemory
#define MoveMemory(d,s,l) memmove((d),(s),(l))
#endif
#ifndef FillMemory
#define FillMemory(d,l,f) memset((d),(f),(l))
#endif

#endif /* _JX3_COMPAT_WINDOWS_H */

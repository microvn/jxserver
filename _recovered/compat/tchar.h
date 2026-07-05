/* Linux compat <tchar.h> — ANSI (non-UNICODE) build: TCHAR == char. */
#ifndef _JX3_COMPAT_TCHAR_H
#define _JX3_COMPAT_TCHAR_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _T
#define _T(x)   x
#define __T(x)  x
#define TEXT(x) x
#endif

typedef char TCHAR;

#define _tcslen    strlen
#define _tcscpy    strcpy
#define _tcsncpy   strncpy
#define _tcscat    strcat
#define _tcscmp    strcmp
#define _tcsicmp   strcasecmp
#define _tcsnicmp  strncasecmp
#define _tcsncmp   strncmp
#define _tcschr    strchr
#define _tcsrchr   strrchr
#define _tcsstr    strstr
#define _stprintf  sprintf
#define _sntprintf snprintf
#define _vstprintf vsprintf
#define _vsntprintf vsnprintf
#define _tprintf   printf
#define _ftprintf  fprintf
#define _tfopen    fopen
#define _tcstoul   strtoul
#define _tcstol    strtol
#define _tstoi     atoi
#define _ttoi      atoi
#define _tcsdup    strdup

#endif /* _JX3_COMPAT_TCHAR_H */

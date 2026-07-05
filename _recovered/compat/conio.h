/* Linux compat <conio.h> — console I/O stubs (server barely uses these). */
#ifndef _JX3_COMPAT_CONIO_H
#define _JX3_COMPAT_CONIO_H

#include <stdio.h>

static inline int getch(void)   { return getchar(); }
static inline int _getch(void)  { return getchar(); }
static inline int kbhit(void)   { return 0; }
static inline int _kbhit(void)  { return 0; }

#define cprintf   printf
#define _cprintf  printf
#define clrscr()

#endif /* _JX3_COMPAT_CONIO_H */

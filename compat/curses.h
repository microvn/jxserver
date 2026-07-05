/* Linux compat <curses.h> stub — the recorder tools include curses.h in their
 * non-WIN32 branch only for console color; the server doesn't need real curses. */
#ifndef _JX3_COMPAT_CURSES_H
#define _JX3_COMPAT_CURSES_H
#include <stdio.h>
#define textcolor(x)
#define textbackground(x)
#define gotoxy(x,y)
#define clrscr()
#endif

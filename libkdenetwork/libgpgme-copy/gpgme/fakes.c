#include <config.h>

/** How can this work? The prototypes are not defined anywhere for code using those functions.... */

#ifndef HAVE_STPCPY
#include "stpcpy.c"
#endif

/*#ifndef HAVE_ISASCII
#include "isascii.c"
#endif*/

#ifndef HAVE_PUTC_UNLOCKED
#include "putc_unlocked.c"
#endif

/*#ifndef HAVE_MEMRCHR
#include "memrchr.c"
#endif*/

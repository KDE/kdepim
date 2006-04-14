# Discover the type of the timezone variable,
# set HAVE_TIMEZONE if found for config.h

INCLUDE (CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
int main() { timezone = 1; return 0;}
" 
  HAVE_TIMEZONE)

IF (NOT HAVE_TIMEZONE)
  # Then it's probably this variant, just to be sure
  CHECK_CXX_SOURCE_COMPILES("
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
int main() { const char *p = timezone(0,0); return 0;}
" 
    HAVE_BSD_TIMEZONE)
ENDIF(NOT HAVE_TIMEZONE)

CHECK_CXX_SOURCE_COMPILES("
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
int main() { struct tm tm; tm.tm_gmtoff=1; return 0; }
"
  HAVE_TM_GMTOFF)


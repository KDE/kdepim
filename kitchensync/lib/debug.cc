#include <kcmdlineargs.h>
#include <klocale.h>

int debug_level = 0;
const char *debug_spaces = "                                               ";

static KCmdLineOptions debug_options_[] =
{
	{ "debug", I18N_NOOP("Show call trace during a run"), 0},
	{ 0,0,0 }
} ;

KCmdLineOptions *debug_options = debug_options_;

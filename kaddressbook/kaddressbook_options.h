#ifndef KADDRESSBOOK_OPTIONS_H
#define KADDRESSBOOK_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions kaddressbook_options[] =
{
  { "a", 0 , 0 },
  { "addr <email>", I18N_NOOP( "Shows contact editor with given email address" ), 0 },
  { "uid <uid>", I18N_NOOP( "Shows contact editor with given uid" ), 0 },
  { "editor-only", I18N_NOOP( "Launches in editor only mode" ), 0 },
  { "new-contact", I18N_NOOP( "Launches editor for the new contact" ), 0 },
  { "+[URL]", I18N_NOOP( "Import the given vCard" ), 0 },
  KCmdLineLastOption
};

#endif /* KADDRESSBOOK_OPTIONS_H */


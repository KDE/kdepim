#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>

#include "DefinitionEditor.h"

static const char * description =
  I18N_NOOP("KDE addressbook definition editor");

static const char * version = "v0.1";

  int
main(int argc, char ** argv)
{
  KAboutData about
    (
     "kab_definition_editor",
     I18N_NOOP("KDE addressbook definition editor"),
     version,
     description,
     KAboutData::License_GPL,
     "(C) 2001 Rik Hemsley"
    );

  about.addAuthor("Rik Hemsley", 0, "rik@kde.org");

	KCmdLineArgs::init(argc, argv, &about, true);

  KApplication * app = new KApplication;

  DefinitionEditor * de = new DefinitionEditor;

  app->setMainWidget(de);

  de->show();

  return app->exec();
}


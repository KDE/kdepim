#include <kapp.h>
#include <kcmdlineargs.h>

#include "DefinitionReader.h"

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "testparser", "testparser", "testparser");

  new KApplication;

  DefinitionReader reader("test_kab_definition");

  reader.parse();

  bool success = reader.success();

  if (!success)
  {
    qDebug("Parsing failed");
    return 1;
  }

  qDebug("Parsing succeded");

  FormatSpec spec = reader.spec();

  for (FormatSpec::ConstIterator it(spec.begin()); it != spec.end(); ++it)
  {
    FieldFormat ff(*it);

    qDebug("------------------------ Field def --------------------------");
    qDebug("Name: %s", ff.name().ascii());
    qDebug("Mime type: %s/%s", ff.type().ascii(), ff.subType().ascii());
    qDebug("Unique: %s", ff.unique() ? "true" : "false");
  }
}


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

  FormatDefinition def = reader.definition();

  QValueList<FieldFormat> l(def.fieldFormatList());

  QValueList<FieldFormat>::ConstIterator it;

  for (it = l.begin(); it != l.end(); ++it)
  {
    FieldFormat ff(*it);

    qDebug("------------------------ Field def --------------------------");
    qDebug("Name: %s", ff.name().ascii());
    qDebug("Mime type: %s/%s", ff.type().ascii(), ff.subType().ascii());
    qDebug("Unique: %s", ff.unique() ? "true" : "false");
  }
}


#include <kapp.h>
#include <kcmdlineargs.h>

#include <kab2/FormatDefinition.h>

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "testparser", "testparser", "testparser");

  new KApplication;

  KAB::FormatDefinition fd("test_kab_definition");

  if (!fd)
  {
    qDebug("Parsing failed");
    return 1;
  }

  qDebug("Parsing succeded");

  QValueList<KAB::FieldFormat> l(fd.fieldFormatList());

  QValueList<KAB::FieldFormat>::ConstIterator it;

  for (it = l.begin(); it != l.end(); ++it)
  {
    KAB::FieldFormat ff(*it);

    qDebug("------------------------ Field def --------------------------");
    qDebug("Name: %s", ff.name().ascii());
    qDebug("Mime type: %s/%s", ff.type().ascii(), ff.subType().ascii());
    qDebug("Unique: %s", ff.unique() ? "true" : "false");
  }
}


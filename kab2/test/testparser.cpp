#include <kapp.h>
#include <kcmdlineargs.h>

#include "FormatHandler.h"

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "testparser", "testparser", "testparser");

  new KApplication;

  FormatHandler handler;

  QFile f("testing.kabformat");

  QXmlInputSource source(f);

  QXmlSimpleReader reader;

  reader.setContentHandler(&handler);

  bool ok = reader.parse(source);

  f.close();

  if (!ok)
  {
    qWarning("Parse error");
    return 1;
  }
  else
  {
    qDebug("Parsing ok");

    FormatSpec spec(handler.spec());

    for (FormatSpec::ConstIterator it(spec.begin()); it != spec.end(); ++it)
    {
      FieldFormat ff(*it);

      qDebug
        (
         "Field: name=`%s' unique=%s type=`%s' subType=`%s'",
         ff.name().latin1(),
         ff.unique() ? "true" : "false",
         ff.type().latin1(),
         ff.subType().latin1()
        );
    }
  }
}


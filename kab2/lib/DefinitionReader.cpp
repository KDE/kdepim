#include <qdom.h>
#include <qfile.h>

#include "FormatSpec.h"
#include "DefinitionReader.h"

DefinitionReader::DefinitionReader(const QString & filename)
  : filename_(filename),
    success_(false)
{
}

DefinitionReader::~DefinitionReader()
{
}

  void
DefinitionReader::parse()
{
  success_ = false;

  QDomDocument doc;

  QFile f(filename_);

  if (!f.open(IO_ReadOnly))
    return;

  if (!doc.setContent(&f))
  {
    f.close();
    return;
  }

  f.close();

  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();

  FormatSpec spec;

  while (!n.isNull())
  {
    QDomElement e = n.toElement();

    if (!e.isNull())
    {
      if (e.tagName() == "kab-def:field")
      {
        FieldFormat ff;

        if (e.hasAttribute("name"))
          ff.setName(e.attribute("name"));

        if (e.hasAttribute("mimetype"))
        {
          QString mimetype = e.attribute("mimetype");

          int sep = mimetype.find('/');

          if (-1 == sep)
            ff.setType(mimetype);

          else
          {
            ff.setType(mimetype.left(sep));
            ff.setSubType(mimetype.mid(sep + 1));
          }
        }

        ff.setUnique("true" == e.attribute("unique"));

        spec << ff;
      }
    }

    n = n.nextSibling();
  }

  spec_ = spec;
  success_ = true;
}


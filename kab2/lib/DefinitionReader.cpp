#include <qdom.h>
#include <qfile.h>

#include "FormatDefinition.h"
#include "DefinitionReader.h"

namespace KAB
{
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

    FormatDefinition def;

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

          def.add(ff);
        }
      }

      n = n.nextSibling();
    }

    def_ = def;
    success_ = true;
  }
}


#include <kab2/FormatDefinition.h>

namespace KAB
{
  FormatDefinition::FormatDefinition()
  {
    // Empty.
  }

  FormatDefinition::FormatDefinition(const QString & xml)
  {
    QDomDocument doc;

    if (!doc.setContent(xml))
      return;

    QDomElement docElem = doc.documentElement();

    setName(docElem.attribute("name"));

    QDomNode n = docElem.firstChild();

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

          add(ff);
        }
      }

      n = n.nextSibling();
    }
  }

  FormatDefinition::~FormatDefinition()
  {
    // Empty.
  }

    bool
  FormatDefinition::isNull() const
  {
    return !name_;
  }

    void
  FormatDefinition::setName(const QString & s)
  {
    name_ = s;
  }

    QString
  FormatDefinition::name() const
  {
    return name_;
  }

    bool
  FormatDefinition::add(const FieldFormat & ff)
  {
    QValueList<FieldFormat>::ConstIterator it;

    for (it = formatList_.begin(); it != formatList_.end(); ++it)
    {
      if ((*it).name() == ff.name())
        return false;
    }

    formatList_.append(ff);
    return true;
  }

    bool
  FormatDefinition::remove(const QString & name)
  {
    QValueList<FieldFormat>::ConstIterator it;

    for (it = formatList_.begin(); it != formatList_.end(); ++it)
    {
      if ((*it).name() == name)
        return true;
    }

    return false;
  }

    bool
  FormatDefinition::replace(const FieldFormat & ff)
  {
    QValueList<FieldFormat>::Iterator it;

    for (it = formatList_.begin(); it != formatList_.end(); ++it)
    {
      if ((*it).name() == ff.name())
      {
        formatList_.remove(it);
        formatList_.append(ff);
        return true;
      }
    }

    formatList_.append(ff);
    return false;
  }

    FieldFormat
  FormatDefinition::fieldFormat(const QString & name) const
  {
    QValueList<FieldFormat>::ConstIterator it;

    for (it = formatList_.begin(); it != formatList_.end(); ++it)
    {
      if ((*it).name() == name)
        return *it;
    }

    return FieldFormat();
  }

    QValueList<FieldFormat>
  FormatDefinition::fieldFormatList() const
  {
    return formatList_;
  }

    bool
  FormatDefinition::contains(const QString & name) const
  {
    return !(fieldFormat(name).isNull());
  }

    void
  FormatDefinition::insertInDomTree(QDomNode & parent, QDomDocument & doc) const
  {
    QDomElement e = doc.createElement("kab-definition");

    QValueList<FieldFormat>::ConstIterator it;

    for (it = formatList_.begin(); it != formatList_.end(); ++it)
      (*it).insertInDomTree(e, doc);

    parent.appendChild(e);
  }

    QString
  FormatDefinition::toXML() const
  {
    QDomDocument doc;

    insertInDomTree(doc, doc);

    return doc.toString();
  }

    QDataStream &
  operator << (QDataStream & str, const FormatDefinition & fd)
  {
    str << fd.name_ << fd.formatList_;

    return str;
  }

    QDataStream &
  operator >> (QDataStream & str, FormatDefinition & fd)
  {
    str >> fd.name_ >> fd.formatList_;

    return str;
  }
}


#include <kdatastream.h>

#include <kab2/FieldFormat.h>

namespace KAB
{
  FieldFormat::FieldFormat
    (
     const QString & name,
     const QString & caption,
     const QString & mimeType,
     const QString & mimeSubType,
     bool list,
     bool unique
    )
    : name_     (name),
      caption_  (caption),
      type_     (mimeType),
      subType_  (mimeSubType),
      list_     (list),
      unique_   (unique)
    {
  }

  FieldFormat::FieldFormat()
    : unique_(false)
  {
  }
  
  FieldFormat::~FieldFormat()
  {
  }

    bool
  FieldFormat::isNull() const
  {
    return name_.isEmpty() || caption_.isEmpty();
  }

    void
  FieldFormat::setName(const QString & s)
  {
    name_ = s;
  }

    void
  FieldFormat::setCaption(const QString & s)
  {
    caption_ = s;
  }

    void
  FieldFormat::setUnique(bool b)
  {
    unique_= b;
  }

    void
  FieldFormat::setList(bool b)
  {
    list_ = b;
  }

    void
  FieldFormat::setType(const QString & s)
  {
    type_ = s;
  }

    void
  FieldFormat::setSubType(const QString & s)
  {
    subType_ = s;
  }

    QString
  FieldFormat::name() const
  {
    return name_;
  }

    QString
  FieldFormat::caption() const
  {
    return caption_;
  }

    bool
  FieldFormat::list() const
  {
    return list_;
  }

    bool
  FieldFormat::unique() const
  {
    return unique_;
  }

    QString
  FieldFormat::type() const
  {
    return type_;
  }

    QString
  FieldFormat::subType() const
  {
    return subType_;
  }

    void
  FieldFormat::insertInDomTree(QDomNode & parent, QDomDocument & doc) const
  {
    QDomElement e = doc.createElement("field");

    e.setAttribute("name",     name_);
    e.setAttribute("caption",  caption_);
    e.setAttribute("mimetype", type_ + "/" + subType_);
    e.setAttribute("list",     list_);
    e.setAttribute("unique",   unique_);

    parent.appendChild(e);
  }

    QDataStream &
  operator << (QDataStream & str, const FieldFormat & ff)
  {
    str
      << ff.name_
      << ff.caption_
      << ff.type_
      << ff.subType_ 
      << ff.list_ 
      << ff.unique_;

    return str;
  }

    QDataStream &
  operator >> (QDataStream & str, FieldFormat & ff)
  {
    str
      >> ff.name_
      >> ff.caption_
      >> ff.type_
      >> ff.subType_ 
      >> ff.list_ 
      >> ff.unique_;

    return str;
  }
}


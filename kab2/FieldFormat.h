#ifndef FIELD_FORMAT_H
#define FIELD_FORMAT_H

#include <qdom.h>
#include <qstring.h>

namespace KAB
{
  class FieldFormat
  {
    public:

      FieldFormat
        (
         const QString & name,
         const QString & caption,
         const QString & mimeType = QString::null,
         const QString & mimeSubType = QString::null,
         bool list = false,
         bool unique = true
        );

      FieldFormat();

      virtual ~FieldFormat();

      bool isNull() const;
      bool operator ! () const { return isNull(); }

      void setName    (const QString & s);
      void setCaption (const QString & s);
      void setUnique  (bool b);
      void setList    (bool b);
      void setType    (const QString & s);
      void setSubType (const QString & s);

      QString name()    const;
      QString caption() const;
      bool    list()    const;
      bool    unique()  const;
      QString type()    const;
      QString subType() const;

      void insertInDomTree(QDomNode & parent, QDomDocument & doc) const;

      friend QDataStream & operator << (QDataStream &, const FieldFormat &);
      friend QDataStream & operator >> (QDataStream &, FieldFormat &);

    private:

      QString name_;
      QString caption_;
      QString type_;
      QString subType_;

      bool list_;
      bool unique_;
  };

} // End namespace KAB.

#endif

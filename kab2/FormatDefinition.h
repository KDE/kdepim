#ifndef FORMAT_SPEC_H
#define FORMAT_SPEC_H

#include <qdom.h>

#include <qvaluelist.h>

#include "FieldFormat.h"

namespace KAB
{
  class FormatDefinition
  {
    public:

      FormatDefinition();
      FormatDefinition(const QString & xml);

      virtual ~FormatDefinition();

      bool isNull() const;
      bool operator ! () const { return isNull(); }

      void setName(const QString &);
      QString name() const;

      bool add(const FieldFormat &);
      bool remove(const QString & name);
      bool replace(const FieldFormat &);

      bool contains(const QString & name) const;

      FieldFormat fieldFormat(const QString & name) const;

      QValueList<FieldFormat> fieldFormatList() const;

      void insertInDomTree(QDomNode & parent, QDomDocument & doc) const;

      QString toXML() const;

      friend QDataStream & operator << (QDataStream &, const FormatDefinition&);
      friend QDataStream & operator >> (QDataStream &, FormatDefinition &);

    private:

      QString name_;

      QValueList<FieldFormat> formatList_;
  };

} // End namespace KAB.

#endif


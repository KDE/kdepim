#ifndef FORMAT_SPEC_H
#define FORMAT_SPEC_H

#include <qvaluelist.h>

#include "FieldFormat.h"

namespace KAB
{
  class FormatDefinition
  {
    public:

      FormatDefinition();
      virtual ~FormatDefinition();

      bool add(const FieldFormat &);
      bool remove(const QString & name);
      bool replace(const FieldFormat &);

      FieldFormat fieldFormat(const QString & name) const;

      QValueList<FieldFormat> fieldFormatList() const;

    private:

      QValueList<FieldFormat> formatList_;
  };
}

#endif


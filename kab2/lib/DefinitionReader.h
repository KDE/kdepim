#ifndef DEFINITION_READER_H
#define DEFINITION_READER_H

#include "FormatDefinition.h"

namespace KAB
{
  class DefinitionReader
  {
    public:

      DefinitionReader(const QString & filename);

      virtual ~DefinitionReader();

      void parse();

      bool success() const
      {
        return success_;
      }

      FormatDefinition definition() const
      {
        return def_;
      }

    private:

      QString filename_;
      bool success_;
      FormatDefinition def_;
  };
}

#endif

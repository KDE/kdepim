#ifndef DEFINITION_READER_H
#define DEFINITION_READER_H

#include "FormatSpec.h"

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

    FormatSpec spec() const
    {
      return spec_;
    }

  private:

    QString filename_;
    bool success_;
    FormatSpec spec_;
};

#endif

#include "FormatDefinition.h"

namespace KAB
{
  FormatDefinition::FormatDefinition()
  {
    // Empty.
  }

  FormatDefinition::~FormatDefinition()
  {
    // Empty.
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
}


#ifndef FIELD_FORMAT_H
#define FIELD_FORMAT_H

#include <qstring.h>

class FieldFormat
{
  public:

    FieldFormat
      (
       const QString & name,
       const QString & caption,
       const QString & mimeType,
       const QString & mimeSubType,
       bool list = false,
       bool unique = true
      )
      : name_(name),
        caption_(caption),
        type_(mimeType),
        subType_(mimeSubType),
        list_(list),
        unique_(unique)
    {
    }

    FieldFormat()
      : unique_(false)
    {
    }

    virtual ~FieldFormat()
    {
    }

    bool isNull() const
    {
      return name_.isEmpty() || caption_.isEmpty();
    }

    void setName    (const QString & s) { name_     = s; }
    void setCaption (const QString & s) { caption_  = s; }
    void setUnique  (bool b)            { unique_   = b; }
    void setList    (bool b)            { list_     = b; }
    void setType    (const QString & s) { type_     = s; }
    void setSubType (const QString & s) { subType_  = s; }

    QString name()    const { return name_;     }
    QString caption() const { return caption_;  }
    bool    list()    const { return list_;     }
    bool    unique()  const { return unique_;   }
    QString type()    const { return type_;     }
    QString subType() const { return subType_;  }

  private:

    QString name_;
    QString caption_;
    QString type_;
    QString subType_;

    bool list_;
    bool unique_;
};

#endif

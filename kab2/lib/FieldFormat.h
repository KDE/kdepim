#ifndef FIELD_FORMAT_H
#define FIELD_FORMAT_H

class FieldFormat
{
  public:

    FieldFormat()
      : unique_(false)
    {
    }

    virtual ~FieldFormat()
    {
    }

    void setName    (const QString & s) { name_     = s; }
    void setUnique  (bool b)            { unique_   = b; }
    void setType    (const QString & s) { type_     = s; }
    void setSubType (const QString & s) { subType_  = s; }

    QString name()    const { return name_;     }
    bool    unique()  const { return unique_;   }
    QString type()    const { return type_;     }
    QString subType() const { return subType_;  }

  private:

    bool unique_;
    QString name_;
    QString type_;
    QString subType_;
};

#endif

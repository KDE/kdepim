#ifndef KAB_FIELD_H
#define KAB_FIELD_H

#include <qvaluelist.h>
#include <qstring.h>


namespace KAB
{

class Field
{
  public:

    /**
     * Create a field with no name, MIME type text/plain and no value.
     */
    Field()
    {
      // Empty.
    }
    
    Field(const Field & v)
      : name_   (v.name_),
        type_   (v.type_),
        subType_(v.subType_),
        data_   (v.data_)
    {
      // Empty.
    }
 
    /**
     * Create a field with the specified name, value and MIME type.
     * 
     * If you leave type empty, it should be assumed that it is 'text'.
     * If you leave subType empty, it should be assumed that it is 'plain'.
     */
    Field(
        const QString &     name,
        const QByteArray &  value,
        const QString &     type    = QString::null,
        const QString &     subType = QString::null)
      :
        name_(name),
        type_(type),
        subType_(subType),
        data_(value)
    {
      // Empty.
    }
    
    virtual ~Field()
    {
      // Empty.
    }      
 
    Field & operator = (const Field & v)
    {
      if (this == &v) return *this;

      name_     = v.name_;
      type_     = v.type_;
      subType_  = v.subType_;
      data_     = v.data_;
      
      return *this;
    }
    
    bool operator == (const Field & f) const
    {
      return (
        (name_    == f.name_    ) &&
        (type_    == f.type_    ) &&
        (subType_ == f.subType_ ) &&
        (data_    == f.data_    ));
    } 
 
    QString     name()    const { return name_;     }
    QString     type()    const { return type_;     }
    QString     subType() const { return subType_;  }
    QByteArray  data()    const { return data_;     }
    
    void setName    (const QString    & n) { name_    = n; }
    void setType    (const QString    & t) { type_    = t; }
    void setSubType (const QString    & s) { subType_ = s; }
    void setData    (const QByteArray & a) { data_    = a; }
    
    virtual void save(QDataStream & str);
    virtual void load(QDataStream & str);

  private:
    
    QString    name_;
    QString    type_;
    QString    subType_;
    QByteArray data_;
};

typedef QValueList<Field> FieldList;

} // End namespace KAB

#endif


#include <qstring.h>

#include <KabSubValue.h>

#ifndef KAB_FIELD_H
#define KAB_FIELD_H

namespace KAB
{

class Field
{
  public:

    Field()
    {
      // Empty.
    }
    
    Field(const Field & v)
      : name_         (v.name_        ),
        subValueList_ (v.subValueList_)
    {
      // Empty.
    }
    
    ~Field()
    {
      // Empty.
    }      
 
    Field & operator = (const Field & v)
    {
      if (this == &v) return *this;

      name_         = v.name_;
      subValueList_ = v.subValueList_;
      
      return *this;
    }
    
    bool operator == (const Field & v) const
    {
      return ((name_ == v.name_) && (subValueList_ == v.subValueList_));
    } 
        
    QString       name()          const { return name_;         }
    SubValueList  subValueList()  const { return subValueList_; }
    
    void  setName         (const QString      & n) { name_ = n;          }
    void  setSubValueList (const SubValueList & l) { subValueList_ = l;  }
 
    friend QDataStream & operator << (QDataStream &, const Field &);
    friend QDataStream & operator >> (QDataStream &, Field &);
    
  private:
    
    QString       name_;
    SubValueList  subValueList_;
};
  
  QDataStream &
operator << (QDataStream & str, const Field & f)
{
  str << f.name_ << f.subValueList_;
  return str;
}

  QDataStream &
operator >> (QDataStream & str, Field & f)
{
  str >> f.name_ >> f.subValueList_;
  return str;
}

} // End namespace KAB

#endif


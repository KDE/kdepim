#include <qstring.h>

#ifndef KAB_EMAILADDRESS_H
#define KAB_EMAILADDRESS_H

namespace KAB
{

class EmailAddress
{
  public:
 
    EmailAddress()
    {
      // Empty.
    }
    
    EmailAddress(const EmailAddress & a)
      : name_   (a.name_),
        domain_ (a.domain_)
    {
      // Empty.
    }
 
    ~EmailAddress()
    {
      // Empty.
    }
    
    EmailAddress & operator = (const EmailAddress & a)
    {
      if (this == &a) return *this;
      
      name_   = a.name_;
      domain_ = a.domain_;
      
      return *this; 
    }
    
    bool operator == (const EmailAddress & a) const
    {
      return ((name_ == a.name_) && (domain_ == a.domain_));
    }
    
    QString name()    const { return name_;   }
    QString domain()  const { return domain_; }
    
    void setName  (const QString & s) { name_   = s; }
    void setDomain(const QString & s) { domain_ = s; }

    friend QDataStream & operator << (QDataStream &, const EmailAddress &);
    friend QDataStream & operator >> (QDataStream &, EmailAddress &);

  private:

    QString name_;
    QString domain_;
};
    
  QDataStream &
operator << (QDataStream & str, const EmailAddress & a)
{
  str << a.name_ << a.domain_;
  return str;
}
  
  QDataStream &
operator >> (QDataStream & str, EmailAddress & a)
{
  str >> a.name_ >> a.domain_;
  return str;
}

} // End namespace KAB

#endif


#include <qstring.h>

#ifndef KAB_TALKADDRESS_H
#define KAB_TALKADDRESS_H

namespace KAB
{

class TalkAddress
{
  public:
 
    TalkAddress()
    {
      // Empty.
    }
    
    TalkAddress(const TalkAddress & a)
      : name_   (a.name_  ),
        domain_ (a.domain_)
    {
      // Empty.
    }
 
    ~TalkAddress()
    {
      // Empty.
    }
    
    TalkAddress & operator = (const TalkAddress & a)
    {
      if (this == &a) return *this;
      
      name_   = a.name_;
      domain_ = a.domain_;
      
      return *this; 
    }
    
    bool operator == (const TalkAddress & a) const
    {
      return ((name_ == a.name_) && (domain_ == a.domain_));
    }
    
    QString name()    const { return name_;   }
    QString domain()  const { return domain_; }
    
    void setName  (const QString & s) { name_   = s; }
    void setDomain(const QString & s) { domain_ = s; }
    
    friend QDataStream & operator << (QDataStream &, const TalkAddress &);
    friend QDataStream & operator >> (QDataStream &, TalkAddress &);
    
  private:
    
    QString name_;
    QString domain_;
};

  QDataStream &
operator << (QDataStream & str, const TalkAddress & a)
{
  str << a.name_ << a.domain_;
  return str;
}

  QDataStream &
operator >> (QDataStream & str, TalkAddress & a)
{
  str >> a.name_ >> a.domain_;
  return str;
}


} // End namespace KAB

#endif


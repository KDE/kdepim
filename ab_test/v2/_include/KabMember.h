#include <KabEntity.h>
#include <KabComms.h>

#ifndef KAB_MEMBER_H
#define KAB_MEMBER_H

namespace KAB
{

class Member : public Entity
{
  public:

    Member(const QString & type, AddressBook & pab, const QString & name)
      : Entity(type, pab, name)
    {
      // Empty.
    }
    
    Member(const Member & m)
      : Entity        (m              ),
        contactInfo_  (m.contactInfo_ )
    {
      // Empty.
    }
 
    ~Member()
    {
      // Empty.
    }
    
    Member & operator = (const Member & m)
    {
      if (this == &m) return *this;
      
      contactInfo_         = m.contactInfo_;
      xValues_             = m.xValues_;
      Entity::operator = (m);
 
      return *this; 
    }
    
    bool operator == (const Member & m) const
    {
      return (
        (contactInfo_   == m.contactInfo_ ) &&
        (xValues_       == m.xValues_     ));
    }
    
    Comms         contactInfo()   const { return contactInfo_;  }

    void setContactInfo   (const Comms          & c)
    { touch(); contactInfo_ = c; }
    
  protected:
    
    Comms         contactInfo_;
};

} // End namespace KAB

#endif


#include <KabEnum.h>
#include <KabEntity.h>
#include <KabComms.h>

#ifndef KAB_MEMBER_H
#define KAB_MEMBER_H

namespace KAB
{

class Member : public Entity
{
  public:

    Member(EntityType type)
      : Entity(type)
    {
    }
    
    Member(EntityType type, AddressBook & pab, const QString & name)
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

    void setContactInfo   (const Comms & c)
    { touch(); contactInfo_ = c; }

    friend QDataStream & operator << (QDataStream &, const Member &);
    friend QDataStream & operator >> (QDataStream &, Member &);
    
  protected:
    
    Comms         contactInfo_;
};

  QDataStream &
operator << (QDataStream & str, const Member & m)
{
  str << m.contactInfo_;
  operator << (str, *((Entity *)&m));
  return str;
}

  QDataStream &
operator >> (QDataStream & str, Member & m)
{
  str >> m.contactInfo_;
  operator >> (str, *((Entity *)&m));
  return str;
}

} // End namespace KAB

#endif


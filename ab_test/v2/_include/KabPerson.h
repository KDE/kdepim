#include <qstring.h>

#include <KabEnum.h>
#include <KabMember.h>
#include <KabPersonalName.h>

#ifndef KAB_PERSON_H
#define KAB_PERSON_H

namespace KAB
{

class Person : public Member
{ 
  public:
    
    Person()
      : Member(EntityTypePerson)
    {
    }

    Person(AddressBook & pab, const QString & name)
      : Member(EntityTypePerson, pab, name)
    {
      // Empty.
    }
    
    Person(const Person & p)
      : Member      (p),
        pname_      (p.pname_   ),
        comms_      (p.comms_   ),
        gender_     (p.gender_  ),
        notes_      (p.notes_   )
    {
      // Empty.
    }
 
    ~Person()
    {
      // Empty.
    }
    
    Person & operator = (const Person & p)
    {
      if (this == &p) return *this;

      pname_      = p.pname_;
      comms_      = p.comms_;
      gender_     = p.gender_;
      notes_      = p.notes_;
      
      Entity::operator = (p);
    
      return *this; 
    }
    
    bool operator == (const Person & p) const
    {
      return (
        (pname_   == p.pname_   ) &&
        (comms_   == p.comms_   ) &&
        (gender_  == p.gender_  ) &&
        (notes_   == p.notes_   ) &&
        (xValues_ == p.xValues_ ));
    }
    
    PersonalName    pname()       const { return pname_;  }
    Comms           contactInfo() const { return comms_;  }
    Gender          gender()      const { return gender_; }
    QString         notes()       const { return notes_;  }
 
    void setPersonalName(const PersonalName   & n)
    { touch(); pname_ = n; }
    
    void setContactInfo (const Comms          & c)
    { touch(); comms_ = c; }
    
    void setGender      (const Gender         & g)
    { touch(); gender_ = g; }
    
    void setNotes       (const QString        & s)
    { touch(); notes_ = s; }
    
    friend QDataStream & operator << (QDataStream &, const Person &);
    friend QDataStream & operator >> (QDataStream &, Person &);

  private:
 
    PersonalName    pname_;
    Comms           comms_;
    Gender          gender_;
    QString         notes_;
};

  QDataStream &
operator << (QDataStream & str, const Person & p)
{
  str << p.pname_ << p.comms_ << (int)p.gender_ << p.notes_;
  operator << (str, *((Member *)&p));
  return str;
}

  QDataStream &
operator >> (QDataStream & str, Person & p)
{
  str >> p.pname_ >> p.comms_;
  int i;
  str >> i;
  p.gender_ = (Gender)i;
  str >> p.notes_;
  operator >> (str, *((Member *)&p));
  return str;
}


} // End namespace KAB

#endif


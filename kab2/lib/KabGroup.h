#ifndef KAB_GROUP_H
#define KAB_GROUP_H

#include <qstring.h>
#include <qdatastream.h>

#include <KabEntity.h>


namespace KAB
{
  
class Group : public Entity
{
  public:
    
    Group()
      : Entity(EntityTypeGroup)
    {
    }
 
    Group(const QString & name)
      : Entity(name, EntityTypeGroup)
    {
      // Empty.
    }
 
    Group(const Group & o)
      : Entity            (o),
        members_          (o.members_)
    {
      // Empty.
    }
 
    ~Group()
    {
      // Empty.
    }
    
    Group & operator = (const Group & o)
    {
      if (this == &o) return *this;
      members_ = o.members_;
      Entity::operator = (o);
      
      return *this; 
    }
    
    bool operator == (const Group & o) const
    {
      return (members_ == o.members_);
    }
    
    EntityRefList   members() const
    { return members_; }
    
    void setMemberList    (const EntityRefList    & l)
    { touch(); members_ = l; }
    
    void addMember        (const EntityRef        & m)
    { touch(); members_.append(m); }
    
    void addMember        (const Entity           & m)
    { touch(); members_.append(m.id()); }
    
    virtual void save(QDataStream &);
    virtual void load(QDataStream &);
    
  private:
    
    EntityRefList members_;
};
    
} // End namespace KAB

#endif


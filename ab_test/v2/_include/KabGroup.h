#include <KabEnum.h>
#include <KabMember.h>

#ifndef KAB_GROUP_H
#define KAB_GROUP_H

namespace KAB
{

class Group : public Member
{
  public:
    
    Group()
      : Member(EntityTypeGroup)
    {
    }
 
    Group(const QString & name)
      : Member(EntityTypeGroup, name)
    {
      // Empty.
    }
 
    Group(const Group & o)
      : Member            (o),
        subGroupRefList_  (o.subGroupRefList_),
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
      
      subGroupRefList_  = o.subGroupRefList_;
      members_          = o.members_;

      Entity::operator = (o);
      
      return *this; 
    }
    
    bool operator == (const Group & o) const
    {
      return (
        (subGroupRefList_ == o.subGroupRefList_  ) &&
        (xValues_         == o.xValues_         ));
    }
    
    GroupRefList    subGroupList()  const { return subGroupRefList_;  }
    MemberRefList   members()       const { return members_;          }
    
    void setMembers       (const MemberRefList    & l)
    { touch(); members_ = l; }
    
    void addMember        (const MemberRef        & m)
    { touch(); members_.append(m); }
    
    void addMember        (const Member           & m)
    { touch(); members_.append(m.id()); }
    
    void setSubGroupList  (const GroupRefList     & l)
    { touch(); subGroupRefList_ = l; }
    
    void addSubGroup(const GroupRef & id)
    { touch(); subGroupRefList_.append(id); }
    
    void addSubGroup(const Group & g)
    { touch(); subGroupRefList_.append(g.id()); }
    
    virtual void save(QDataStream & str);
    virtual void load(QDataStream & str);
    
  private:
    
    GroupRefList  subGroupRefList_;
    MemberRefList members_;
};
    
  void
Group::save(QDataStream & str)
{
  Member::save(str);
  str << subGroupRefList_ << members_;
}
    
  void
Group::load(QDataStream & str)
{
  Member::load(str);
  str >> subGroupRefList_ >> members_;
}

} // End namespace KAB

#endif


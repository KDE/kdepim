#include <KabEnum.h>
#include <KabMember.h>

#ifndef KAB_GROUP_H
#define KAB_GROUP_H

namespace KAB
{

class Group : public Member
{
  public:
 
    Group(AddressBook & pab, const QString & name)
      : Member("group", pab, name)
    {
      // Empty.
    }
 
    Group(Group & parentGroup, const QString & name)
      : Member("group", *(parentGroup.addressBook()), name),
        parentGroup_(parentGroup.id())
    {
      // Empty.
    }
    
    Group(const Group & o)
      : Member            (o),
        subGroupRefList_  (o.subGroupRefList_),
        members_          (o.members_   ),
        parentGroup_      (o.parentGroup_)
 
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
      parentGroup_      = o.parentGroup_;

      Entity::operator = (o);
      
      return *this; 
    }
    
    bool operator == (const Group & o) const
    {
      return (
        (subGroupRefList_ == o.subGroupRefList_  ) &&
        (xValues_         == o.xValues_         ));
    }
    
    Group * parent() const
    {
      if (!parentGroup_)
        return 0;
      Entity * e = addressBook()->entity(parentGroup_);
      return e ? (Group *)e : 0;
    }
    GroupRefList   subGroupList() const { return subGroupRefList_;  }
    MemberRefList  members()      const { return members_;          }
    MemberRefList  allMembers()   const
    {
      MemberRefList l(members_);
      GroupRefList::ConstIterator it(subGroupRefList_.begin());
      for (; it != subGroupRefList_.end(); ++it)
        l += ((Group *)(addressBook_->entity(*it)))->allMembers();
      return l;
    }
    
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
    
  private:
    
    GroupRefList  subGroupRefList_;
    MemberRefList members_;
    GroupRef      parentGroup_;
};

} // End namespace KAB

#endif


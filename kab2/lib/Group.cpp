#include <iostream>
#include <KabGroup.h>

using namespace KAB;

  void
Group::save(QDataStream & str)
{
  cerr << "Group::save" << endl;
  Entity::save(str);
  str << members_;
}
    
  void
KAB::Group::load(QDataStream & str)
{
  cerr << "Group::load" << endl;
  Entity::load(str);
  str >> members_;
}



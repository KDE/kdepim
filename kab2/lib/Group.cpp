#include <KabGroup.h>

using namespace KAB;

  void
Group::save(QDataStream & str)
{
  Entity::save(str);
  str << members_;
}
    
  void
KAB::Group::load(QDataStream & str)
{
  Entity::load(str);
  str >> members_;
}



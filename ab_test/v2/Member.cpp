#include <KabMember.h>
#include <KabEntity.h>
#include <KabComms.h>

  void
Member::save(QDataStream & str)
{
  Entity::save(str);
  contactInfo_.save(str);
}

  void
Member::load(QDataStream & str)
{
  Entity::load(str);
  contactInfo_.load(str);
}



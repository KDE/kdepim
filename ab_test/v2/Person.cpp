#include <KabPerson.h>
#include <KabMember.h>
#include <KabPersonalName.h>
#include <KabComms.h>

  void
Person::save(QDataStream & str)
{
  Member::save(str);
  pname_.save(str);
  comms_.save(str);
  str << (int)gender_ << notes_;
}

  void
Person::load(QDataStream & str)
{
  Member::load(str);

  pname_.load(str);
  comms_.load(str);

  int i;
  str >> i;
  gender_ = (Gender)i;
  str >> notes_;
}



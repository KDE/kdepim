#include <KabField.h>

using namespace KAB;

  void
Field::save(QDataStream & str)
{
  str << name_;
  str << type_;
  str << subType_;
  str << data_;
}

  void
Field::load(QDataStream & str)
{
  str >> name_;
  str >> type_;
  str >> subType_;
  str >> data_;
}


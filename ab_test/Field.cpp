#include <iostream.h>
#include <Field.h>

using namespace KAB;

Field::Field()
{
}

Field::Field(const QString & name, const Value & value)
  : name_  (name),
    value_  (value)
{
}

Field::Field(const Field & f)
  : name_  (f.name_),
    value_  (f.value_)
{
}

  void
Field::setName(const QString & s)
{
  name_ = s;
}

  void
Field::setValue(const Value & v)
{
  value_ = v;
}

Field::~Field()
{
}

  QDataStream &
KAB::operator >> (QDataStream & s, Field & f)
{
  s >> f.name_ >> f.value_;
  return s;
}

  QDataStream &
KAB::operator << (QDataStream & s, const Field & f)
{
  s << f.name_ <<  f.value_;
  return s;
}

// vim:ts=2:sw=2:tw=78:

#include <Entry.h>

using namespace KAB;

Entry::Entry()
{
}

Entry::Entry(const QString & name)
  :  name_  (name)
{
}

Entry::~Entry()
{
}

  FieldList
Entry::fields(const QString & fieldName)
{
  FieldList l;
  
  FieldListIterator it(fieldList_.begin());
  
  for (; it != fieldList_.end(); ++it)
    if ((*it).name() == fieldName)
      l.append(*it);
  
  return l;
}

  FieldList
Entry::fields(const QRegExp & expression)
{
  FieldList l;
  
  FieldListIterator it(fieldList_.begin());
  
  for (; it != fieldList_.end(); ++it)
    if (expression.match((*it).name()) != -1)
      l.append(*it);
  
  return l;
}

  FieldList
Entry::fieldsWithValueType(ValueType t)
{
  FieldList l;
  
  FieldListIterator it(fieldList_.begin());
  
  for (; it != fieldList_.end(); ++it)
    if ((*it).typeEnum() == t || t == Any)
      l.append(*it);
  
  return l;
}
  
  FieldList
Entry::fieldsWithValueType(const QString & valueType)
{
  FieldList l;
  
  FieldListIterator it(fieldList_.begin());
  
  for (; it != fieldList_.end(); ++it)
    if ((*it).type() == valueType)
      l.append(*it);
  
  return l;
}

  FieldList
Entry::fieldsWithExtensionValueType()
{
  return fieldsWithValueType(XValue);
}

  FieldList
Entry::fieldsWithStandardValueType()
{
  FieldList l;
  
  FieldListIterator it(fieldList_.begin());
  
  for (; it != fieldList_.end(); ++it)
    if ((*it).type() != XValue)
      l.append(*it);
  
  return l;
}
  
  void
Entry::add(const Field & f)
{
  fieldList_.append(f);
}

  QDataStream &
KAB::operator >> (QDataStream & s, Entry & e)
{
  s >> e.name_ >> e.fieldList_;
  return s;
}

  QDataStream &
KAB::operator << (QDataStream & s, const Entry & e)
{
  s << e.name_ << e.fieldList_;
  return s;
}

// vim:ts=2:sw=2:tw=78:

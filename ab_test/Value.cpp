#include <qdatastream.h>
#include <Value.h>

using namespace KAB;

Value::Value()
  :  typeEnum_(XValue)
{
}
  
Value::Value(const Value & v)
  : data_     (v.data_),
    type_     (v.type_),
    typeEnum_ (v.typeEnum_)
{  
}

Value::Value(const QString & data, ValueType t)
  : data_     (data),
    type_     (valueTypeToString(t)),
    typeEnum_ (t)
{
}

Value::Value(const QString & data, const QString & type = QString::null)
  : data_     (data),
    type_     (type),
    typeEnum_ (typeNameToEnum(type))
{
}

Value::~Value()
{
}

  QDataStream &
KAB::operator >> (QDataStream & s, Value & v)
{
  Q_UINT32 i;
  s >> v.data_ >> v.type_ >> i;
  v.typeEnum_ = (ValueType)i;
  return s;
}

  QDataStream &
KAB::operator << (QDataStream & s, const Value & v)
{
  s << v.data_ << v.type_ << (Q_UINT32)v.typeEnum_;
  return s;
}
  
// vim:ts=2:sw=2:tw=78:

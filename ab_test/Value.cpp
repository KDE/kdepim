#include <qdatastream.h>
#include <Value.h>

using namespace KAB;

Value::Value()
	:	typeEnum_	(XValue)
{
	kabDebug("default ctor");
}
	
Value::Value(const Value & v)
	:	data_		(v.data_),
		type_		(v.type_),
		typeEnum_	(v.typeEnum_)
{	
	kabDebug("copy ctor");
}

Value::Value(const QString & data, ValueType t)
	:	data_		(data),
		type_		(valueTypeToString(t)),
		typeEnum_	(t)
{
	kabDebug("ctor");
}

Value::Value(const QString & data, const QString & type = QString::null)
	:	data_		(data),
		type_		(type),
		typeEnum_	(typeNameToEnum(type))
{
	kabDebug("ctor");
}

Value::~Value()
{
	kabDebug("dtor");
}

	QDataStream &
KAB::operator >> (QDataStream & s, Value & v)
{
	s	>>	v.data_
		>>	v.type_;
	
	Q_UINT32 typeEnum;
	
	s	>> typeEnum;
	
	v.typeEnum_ = (ValueType)typeEnum;
	
	return s;
}

	QDataStream &
KAB::operator << (QDataStream & s, const Value & v)
{
	s	<<	v.data_
		<<	v.type_
		<<	(Q_UINT32)v.typeEnum_;

	return s;
}

	

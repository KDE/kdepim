#include <qstring.h>

#include <Enum.h>

#ifndef KAB_VALUE_H
#define KAB_VALUE_H

namespace KAB
{

class Value
{
	public:
		
		/**
		 * Default ctor.
		 * The value type is initialised to XValue.
		 */
		Value();
	
		/**
		 * Initialised the data and set the value type to the predefined type
		 * given. The string version of the value type is set to the correct
		 * value, or QString::null if it's XValue.
		 */
		Value(const QString & data, ValueType t);
	
		/**
		 * Initialised the data and set the value type to the type given.
		 * The type is looked up to see if it matches one of the standard
		 * types. If not, the ValueType is set to XValue.
		 */
		Value(const QString & data, const QString & type = QString::null);
		
		/**
		 * Copy ctor.
		 */
		Value(const Value &);

		~Value();
		
		/**
		 * Get the data from this value.
		 */
		QString 	data()	  	const	{ return data_;	}
		
		/**
		 * Get the type of this value as a string.
		 * If the type string has not been initialised and the enumeration
		 * value is XValue, this will return QString::null.
		 */
		QString		type()		  const	{ return type_;	}
		
		/**
		 * Get the type of this value. If the type isn't one of the standard
		 * types, returns XValue. 
		 */
		ValueType	typeEnum()	const	{ return typeEnum_; }
		
		/**
		 * Initialise type and data from a QDataStream.
		 */
		friend QDataStream & operator >> (QDataStream &, Value &);
		
		/**
		 * Write to a QDataStream.
		 */
		friend QDataStream & operator << (QDataStream &, const Value &);
	
		/**
		 * @internal
		 */
		const char * className() const { return "Value"; }
		
	private:
		
		QString		data_;
		QString		type_;
		ValueType	typeEnum_;
};

}

#endif
// vim:ts=2:sw=2:tw=78:

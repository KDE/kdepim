#include <qvaluelist.h>
#include <qstring.h>

#include <Value.h>

#ifndef KAB_FIELD_H
#define KAB_FIELD_H

namespace KAB
{

class Field
{
	public:
		
		/**
		 * Default ctor.
		 */
		Field();

		/**
		 * Initialises name and value from the given args.
		 */
		Field(const QString & name, const Value & value);

		/**
		 * Copy ctor.
		 */
		Field(const Field &);
		
		~Field();
		
		/**
		 * Assignment operator.
		 */
		Field &	operator = (const Field &);
		/**
		 * Comparison operator.
		 */
		bool  	operator == (const Field &);
	
		/**
		 * Get the name of this field.
		 */
		QString 	name()		const	{ return name_;				}
		/**
		 * Get the type of the value of this field.
		 */
		QString		type()		const	{ return value_.type();		}
		/**
		 * Get the type of this field as a ValueType.
		 * @returns one of the value types enumerated in Enum.h. If the value
		 * type isn't one of the standard types, returns XValue.
		 */
		ValueType	typeEnum()	const	{ return value_.typeEnum();	}
		/**
		 * Get the data part of the value of this field.
		 */
		QString	 	data()		const	{ return value_.data();		}
		/**
		 * Get the value of this field.
		 */
		Value 		value()		const	{ return value_;			}
		
		/**
		 * Set the name of this field.
		 * This will make it impossible to look up in the addressbook if
		 * it's already in there, so get a copy, remove from addressbook,
		 * then re-insert.
		 */
		void setName(const QString &);
		
		/**
		 * Sets the value of this field.
		 */
		void setValue(const Value &);
		
		/**
		 * Initialise name and value from a QDataStream.
		 */
		friend QDataStream & operator >> (QDataStream &, Field &);
		
		/**
		 * Write to a QDataStream.
		 */
		friend QDataStream & operator << (QDataStream &, const Field &);
	
		/**
		 * @internal
		 */
		const char * className() const { return "Field"; }
		
	private:
		
		QString name_;
		Value   value_;
};

typedef QValueList<Field>		        			FieldList;
typedef QValueList<Field>::Iterator	  		FieldListIterator;
typedef QValueList<Field>::ConstIterator	FieldListConstIterator;

}

#endif
// vim:ts=2:sw=2:tw=78:

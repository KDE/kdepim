#include <qstring.h>
#include <qregexp.h>
#include <qvaluelist.h>
#include <qdict.h>

#include <Enum.h>
#include <Field.h>

#ifndef KAB_ENTRY_H
#define KAB_ENTRY_H

namespace KAB
{

class Entry
{
	public:
		
		/**
		 * Default ctor.
		 */
		Entry();
		/**
		 * Constructor that initialises the name.
		 */
		Entry(const QString & name);
    /**
     * dtor.
     */
		~Entry();
		/**
		 * Get the name of this entry.
		 */
		QString		name()    const	{ return name_;		  	}
		/**
		 * Get a copy of the field list.
		 */
		FieldList	fields()  const	{ return fieldList_;	}
		/**
		 * Add a new field.
		 */
		void add(const Field &);
		/**
		 * Find the fields with the specified field name.
		 */
		FieldList	fields(const QString & fieldName);
		/**
		 * Find the fields with the specified field name.
		 */
		FieldList	fields(const QRegExp & expression);
		/**
		 * Find the fields with the specified value type.
		 */
		FieldList	fieldsWithValueType(ValueType t);
		/**
		 * Find the fields with the specified value type.
		 */
		FieldList	fieldsWithValueType(const QString & valueType);
		/**
		 * Find the fields with an extension value type.
		 */
		FieldList	fieldsWithExtensionValueType();
		/**
		 * Find the fields with a standard value type.
		 */
		FieldList	fieldsWithStandardValueType();
		/**
		 * Add fields from a QDataStream.
		 */
		friend QDataStream & operator >> (QDataStream &, Entry &);
		/**
		 * Write all fields to a QDataStream.
		 */
		friend QDataStream & operator << (QDataStream &, const Entry &);
		/**
		 * @internal
		 */
		const char * className() const { return "Entry"; }
		
	private:
		
		QString		name_;
		FieldList	fieldList_;
};

typedef QValueList<Entry>				        	EntryList;
typedef QValueList<Entry>::Iterator		  	EntryListIterator;
typedef QValueList<Entry>::ConstIterator  EntryListConstIterator;

typedef QDict<Entry>				    		EntryDict;
typedef QDictIterator<Entry>				EntryDictIterator;

}

#endif
// vim:ts=2:sw=2:tw=78:

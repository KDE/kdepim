#include <qstringlist.h>
#include <qstring.h>
#include <qregexp.h>
#include <qdict.h>

#include <Enum.h>
#include <Entry.h>
#include <Field.h>
#include <Value.h>

#ifndef KAB_ADDRESSBOOK_H
#define KAB_ADDRESSBOOK_H

namespace KAB
{

class AddressBook
{
  public:
    
    /**
     * Default ctor.
     * Call init() afterwards, before using any methods.
     */
    AddressBook();
    /**
     * Copy ctor.
     */
    AddressBook(const AddressBook &);
    /**
     * xxref.
     */
    AddressBook & operator = (const AddressBook &);
    /**
     * dtor.
     */
    ~AddressBook();
    /**
     * You must call this to initialise the addressbook. Do it after
     * you have created an addressbook and before you use any of
     * its methods.
     */
    void init();
    /**
     * Import entries from another addressbook.
     * @arg format the format the addressbook is stored in. Currently only
     * VCARD and LDIF are recognised.
     * @arg filename where to get the addressbook from.
     * @returns -1 if failed to import, else the number of entries that
     * were imported.
     */
    Q_UINT32  import(const QString & format, const QString & filename);
    /**
     *  Find the entry with the specified name.
     *  @returns 0 if not found, else a pointer to the found entry.
     */
    Entry *   entry(const QString & name);
    /**
     * Find the fields with the specified field name.
     */
    FieldList fields(const QString & fieldName);
    /**
     * Find the entries with the specified field name.
     */
    EntryList entries(const QRegExp & expression);
    /**
     * Find the fields with the specified name.
     */
    FieldList fields(const QRegExp & expression);
    /**
     * Find the fields with the specified value type.
     */
    FieldList fieldsWithValueType(ValueType t);
    /**
     * Find the fields with the specified value type.
     */
    FieldList fieldsWithValueType(const QString & valueType);
    /**
     * Find all extension fields in use.
     */
    FieldList fieldsWithExtensionValueType();
    /**
     * Find all standard fields in use.
     */
    FieldList fieldsWithStandardValueType();
    /**
     * Add a new entry. The entry will be copied so you may delete it.
     * The case where an entry can not be added is one where the name
     * of the entry is empty, so make sure you name your entries.
     * @returns true if the entry could be added, false if not.
     */
    bool add(const Entry &);
    /**
     * Delete an entry.
     * @returns true if the entry existed, false if not.
     */
    bool remove(const QString &);
    /**
     * Replace an entry.
     * If there's no such entry in existence, the given entry is simply added.
     * @returns true if the entry could be replaced, false if not. The only
     * fail condition is that the new entry could not be added. Whether the
     * original could be removed is ignored.
     */
    bool update(const Entry &);
    /**
     * Add entries from a QDataStream.
     */
    friend QDataStream & operator >> (QDataStream &, AddressBook &);
    /**
     * Write all entries to a QDataStream.
     */
    friend QDataStream & operator << (QDataStream &, const AddressBook &);
    /**
     * Write the addressbook to a file.
     * @returns true for success, false for failure.
     */
    bool save(const QString & filename);
    /**
     * Read the addressbook from a file.
     * @returns true for success, false for failure.
     */
    bool load(const QString & filename);
    /**
     * Clear out this addressbook.
     */
    void clear();
    /**
     * @internal
     */
    const char * className() const { return "AddressBook"; }
    
  private:
    
    EntryDict   entryDict_;
};

int (*filterfn) (const char *, KAB::AddressBook *);

typedef   void *            FilterHandle;
typedef   typeof(filterfn)  Filter;

}

#endif
// vim:ts=2:sw=2:tw=78:

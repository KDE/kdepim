/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <dsanders@kde.org>

   License: GNU GPL
*/

#ifndef CONTACTENTRY_H 
#define CONTACTENTRY_H 

#include <qobject.h>
#include <qdict.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qlist.h>
#include <qstringlist.h>
#include <addressbook.h>

/**
 * The ContactEntry class is used to store the current state of an address
 * book entry being edited. It is used in conjunction with ContactDialog,
 * and in doing so it plays the Document part of the View/Document pattern.
 * 
 * An instance of this call is also an Observer emitting the changed() signal 
 * to inform other objects that it has been edited. This allows different
 * widgets showing the same entry field in the corresponding ContactDialog 
 * object to be kept synchronized.
 *
 * This class is just a placeholder until it can be replaced with something
 * more suitable. It is incomplete, for instance no complementary 
 * ContactEntryIterator and ContactEntryList classes has been defined, 
 * spurious load and save methods exist that will have to be removed.
 *
 * Maybe this class is general enough to store PIM entries of all types
 * and not just address book entries. Perhaps this class could be augmented
 * by adding methods for encoding and decoding an object of this type to 
 * vCard or some other format.
 *
 * A couple of conventions are used
 * Fields beginning with the prefix "." won't be saved.
 * Fields beginning with the prefix "X-CUSTOM-" are custom fields.
 *
 * The underlying implementation is based on a QDict<QString> object.
 **/

class ContactEntry;
class KabAPI;
//class AddressBook;
//class AddressBook::Entry;

typedef QDictIterator<ContactEntry> ContactEntryListIterator;

class ContactEntryList
{
public:
  ContactEntryList();
  ~ContactEntryList();

  QString insert( ContactEntry *item );
  void unremove( const QString &key, ContactEntry *item );
  void remove( const QString &key );
  ContactEntry* find( const QString &key );
  void replace( const QString &key, ContactEntry *item );
  QStringList keys();

 protected:
  ContactEntry *KabEntryToContactEntry( AddressBook::Entry entry );
  AddressBook::Entry ContactEntryToKabEntry( ContactEntry *entry, AddressBook::Entry );

  KabAPI *addrBook;
  QStringList removedKeys;
  QDict<ContactEntry> ceDict;
  //  void save();
  //  void load();
};

/*
class ContactEntryList : public QDict<ContactEntry>
{
public:
  ContactEntryList();
  //  ContactEntryList( const QString &filename );
  //  QString key();
  QString insert( ContactEntry *item );
  void unremove( const QString &key, ContactEntry *item );
  void save();
  void load();

protected:
  long kkey;
  QList<ContactEntry> list;
};
*/

class ContactEntry : public QObject
{
    Q_OBJECT

public:
/**
 * Creates a new ContactEntry object.
 */
  ContactEntry();
  ContactEntry( const ContactEntry &r );
  ContactEntry& operator=( const ContactEntry &r );

/**
 * Creates a ContactEntry object from data stored in a textstream.
 * 
 * Arguments:
 *
 * @param the name of the textstream.
 */
  ContactEntry( QTextStream &t );

/**
 * Returns a list of all custom fields, that is those beginning with
 * the prefix "X-CUSTOM-"
 */
  QStringList custom() const;

/**
 * Saves the entry to a text stream with the given filename.
 */
  void save( QTextStream &t );

/**
 * Loads the entry from a text stream with the given filename
 */
  void load( QTextStream &t );

/**
 * Inserts a new key/value pair 
 */
  void insert( const QString key, const QString *value );

/**
 * Updates the value associated with a key. The old value
 * will be deleted.
 */
  void replace( const QString key, const QString *value );  

/**
 * Remove a key and deletes its associated value.
 */ 
  bool remove ( const QString key );

/**
 * Returns a const pointer to the value associated with a key.
 */
  const QString *find ( const QString & key ) const ;

/**
 * Returns a const pointer to the value associated with a key.
 */
  const QString *operator[] ( const QString & key ) const;

/**
 * Cause the changed signal to be emitted.
 */
  void touch();

/**
 * Remove all key/value pairs stored.
 */
  void clear();

signals:
/**
 * Emitted when key/value pair is updated or inserted
 */
  void changed();

private:
  QDict<QString> dict; // This unfortunately doesn't make a good base class
  // It's not derived from QOBject and the majority of methods are not virtual
};

#endif

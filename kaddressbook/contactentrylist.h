/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <sanders@kde.org>

   License: BSD
*/

#ifndef CONTACTENTRYLIST_H
#define CONTACTENTRYLIST_H

#include <qtextstream.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <kabc/addressbook.h>

#include "contactentry.h"


typedef QDictIterator<ContactEntry> ContactEntryListIterator;

class ContactEntryList
{
public:
  ContactEntryList();
  ~ContactEntryList();

  void commit();
  void refresh();
  QString insert( ContactEntry *item );
  void unremove( const QString &key, ContactEntry *item );
  void remove( const QString &key );
  ContactEntry* find( const QString &key );
  /** Removes all items in the trash from the list */
  void emptyTrash();

  void replace( const QString &key, ContactEntry *item );
  QStringList keys() const;
  QDict<ContactEntry> getDict() const;

  KABC::AddressBook *addressBook() { return mAddressBook; }

// protected: // I dont see any reason for these to be protected - they are usefull functions for others
  static ContactEntry *KabEntryToContactEntry( const KABC::Addressee &entry );
  static KABC::Addressee ContactEntryToKabEntry( ContactEntry *entry );
  static void readAddress( KABC::Address ad, ContactEntry *ce, const QString &type );
  static void readPhoneNumber( KABC::Addressee a, int kabcType,
                        ContactEntry *ce, const QString &type );

  static void writeCustom( KABC::Addressee &a, ContactEntry *entry,
                    const QString &type );
  static void writeAddress( KABC::Addressee &a, ContactEntry *entry,
                     const QString &type, int kabcType );
  static void writePhoneNumber( KABC::Addressee &a, ContactEntry *entry,
                         const QString &type, int kabcType );

private:

  KABC::AddressBook *mAddressBook;
  QDict<ContactEntry> ceDict;
};

#endif

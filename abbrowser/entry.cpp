/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <dsanders@kde.org>

    License: GNU GPL
*/

#include "entry.h"
#include <qdict.h>
#include <qfile.h>
#include <klocale.h>

////////////////////////////
// ContactEntryList methods

ContactEntryList::ContactEntryList( const QString &filename )
{
  kkey = 0;
  setAutoDelete( true );
  load( filename );
}

QString ContactEntryList::key()
{
  ++kkey;
  return QString().setNum( kkey );
}

QString ContactEntryList::insert( ContactEntry *item )
{
  QString result = key();
  QDict<ContactEntry>::insert( result, item );
  return result;
}

void ContactEntryList::unremove( const QString &key, ContactEntry *item )
{
  QDict<ContactEntry>::insert( key, item );  
}

void ContactEntryList::save( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_WriteOnly ) )
    return;

  QTextStream t( &f );

  QDictIterator<ContactEntry> it(*this);
  while (it.current()) {
    it.current()->save( t );
    ++it;
  }

  f.close();
}

void ContactEntryList::load( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) )
    return;

  QTextStream t( &f );
  clear();

  while ( !t.eof() ) {
    QDict<ContactEntry>::insert( key(), new ContactEntry( t ));
    // connect up a signal to emit of this guy
  } 

  f.close();
}

////////////////////////
// ContactEntry methods

ContactEntry::ContactEntry() 
{
  dict.setAutoDelete( true );
}

ContactEntry::ContactEntry( const ContactEntry &r )
{
  QDictIterator<QString> it( r.dict );
  
  while ( it.current() ) {
    dict.replace( it.currentKey(), new QString( *it.current() ));
    ++it;
  }
}

ContactEntry& ContactEntry::operator=( const ContactEntry &r )
{
  if (this != &r) {
    dict.clear();
    QDictIterator<QString> it( r.dict );
    
    while ( it.current() ) {
      dict.replace( it.currentKey(), new QString( *it.current() ));
      ++it;
    }
  }
  return *this;
}

ContactEntry::ContactEntry( const QString &filename )
{
  dict.setAutoDelete( true );
  load( filename );
}

ContactEntry::ContactEntry( QTextStream &t )
{
  dict.setAutoDelete( true );
  load( t );
}

QStringList ContactEntry::custom() const
{
  QStringList result;
  QDictIterator<QString> it( dict );

  while ( it.current() ) {
    if (it.currentKey().find( "X-CUSTOM-", 0 ) == 0)
      result << it.currentKey();
    ++it;
  }
  return result;
}

void ContactEntry::save( const QString &filename )
{
  QFile f( filename );
  if ( !f.open( IO_WriteOnly ) )
    return;

  QTextStream t( &f );
  QDictIterator<QString> it( dict );

  while ( it.current() ) {
    if (it.currentKey().find( ".", 0 ) != 0) {
      t << it.currentKey() << "\n";
      t << *it.current() << "\n[EOR]\n";
    }
    ++it;
  }
  t << "[EOS]\n";
  f.close();
}

void ContactEntry::save( QTextStream &t )
{
  QDictIterator<QString> it( dict );

  while ( it.current() ) {
    if (it.currentKey().find( ".", 0 ) != 0) {
      t << it.currentKey() << "\n";
      t << *it.current() << "\n[EOR]\n";
    }
    ++it;
  }
  t << "[EOS]\n";
}

void ContactEntry::load( const QString &filename )
{
  dict.clear();

  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) )
    return;

  QTextStream t( &f );

  while ( !t.eof() ) {
    QString name = t.readLine();
    if (name == "[EOS]")
      break;
    QString tmp = t.readLine();
    QString value = "";
    while (tmp != QString( "\n[EOR]" )) {
      value += tmp;
      tmp = "\n" + t.readLine();
    }
    if ((name != "") && (value != ""))
      dict.replace( name, new QString( value ));
  }
  f.close();
  emit changed();
}

void ContactEntry::load( QTextStream &t )
{
  while (!t.eof()) {
    QString name = t.readLine();
    if (name == "[EOS]")
      break;
    QString tmp = t.readLine();
    QString value = "";
    while (tmp != QString( "\n[EOR]" )) {
      value += tmp;
      tmp = "\n" + t.readLine();
    }
    if ((name != "") && (value != ""))
      dict.replace( name, new QString( value ));
  }
  emit changed();
}

void ContactEntry::insert( const QString key, const QString *item )
{
  if (item && (*item == ""))
    return;
  dict.insert( key, item );
  emit changed();
}

void ContactEntry::replace( const QString key, const QString *item )
{
  QString *current = dict.find( key );
  if (item) {
    if (current) {
      if (*item != *current) {
	if (*item == "")
	  dict.remove( key ); // temporary?
	else
	  dict.replace( key, item );
	emit changed();
      }
    }
    else { // (item && !current)
      dict.replace( key, item );
      emit changed();
    }
  }
  else
    debug( QString( "Error:" ) + 
	   " ContactEntry::replace( const QString, const QString* ) " +
	   "passed null item" );
  /*
  if (item && (*item == ""))
    dict.remove( key );
  else
    dict.replace( key, item );
  emit changed();
  */
}

bool ContactEntry::remove( const QString key )
{
  if (dict.remove( key ))
    emit changed();
}

void ContactEntry::touch()
{
  emit changed();
}

const QString *ContactEntry::find ( const QString & key ) const
{
  return dict.find( key );
}

const QString *ContactEntry::operator[] ( const QString & key ) const
{
  return dict[key];
}

void ContactEntry::clear ()
{
  dict.clear();
  emit changed();
}

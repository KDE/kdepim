/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <dsanders@kde.org>

    License: GNU GPL
*/

#include "contactentry.h"
#include "qdict.h"
#include "qfile.h"
#include "qtextstream.h"

ContactEntry::ContactEntry() 
{
  dict.setAutoDelete( true );
}

ContactEntry::ContactEntry( const QString &filename )
{
  dict.setAutoDelete( true );
  load( filename );
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
  f.close();
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
    QString tmp = t.readLine();
    QString value = "";
    while (tmp != QString( "\n[EOR]" )) {
      value += tmp;
      tmp = "\n" + t.readLine();
    }
    if ((name != "") && (value != ""))
      dict.replace( name, new QString( value ));
    debug( "name" + name + "  value " +  value );
  }
  f.close();
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
  if (item && (*item == ""))
    dict.remove( key );
  else
    dict.replace( key, item );
  emit changed();
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

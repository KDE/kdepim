/*
    ktnefpropertyset.cpp

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "ktnef/ktnefpropertyset.h"
#include "ktnef/ktnefproperty.h"

#include <QList>

#include <kdebug.h>

KTNEFPropertySet::KTNEFPropertySet()
{
}

KTNEFPropertySet::~KTNEFPropertySet()
{
	clear( true );
}

void KTNEFPropertySet::addProperty( int key, int type, const QVariant& value, const QVariant& name, bool overwrite )
{
	QMap<int,KTNEFProperty*>::ConstIterator it = properties_.find( key );
	if ( it != properties_.end() )
	{
		if ( overwrite )
			delete ( *it );
		else
			return;
	}
	KTNEFProperty *p = new KTNEFProperty( key, type, value, name );
	properties_[ p->key() ] = p;
}


QString KTNEFPropertySet::findProp(int key, const QString& fallback, bool upper)
{
  QMap<int,KTNEFProperty*>::Iterator it = properties_.find( key );
  if( properties_.end() != it )
    return upper ? KTNEFProperty::formatValue( (*it)->value(), false ).toUpper()
                 : KTNEFProperty::formatValue( (*it)->value(), false );
  else
    return fallback;
}


QString KTNEFPropertySet::findNamedProp(const QString& name, const QString& fallback, bool upper)
{
  for ( QMap<int,KTNEFProperty*>::Iterator it = properties_.begin();
        it != properties_.end();
        ++it ){
    if ( (*it)->name().isValid() ){
      QString s;
      if ( (*it)->name().type() == QVariant::String )
        s = (*it)->name().toString();
      else
        s = QString().sprintf( "0X%04X", (*it)->name().toUInt() );

      if( s.toUpper() == name.toUpper() ){
        QVariant value = ( *it )->value();
        if( value.type() == QVariant::List ){
          QList<QVariant> l = value.toList();
          s = "";
          for ( QList<QVariant>::ConstIterator lit = l.begin();
                lit != l.end();
                ++lit ){
            if( !s.isEmpty() )
              s += ',';
            s += KTNEFProperty::formatValue( *lit, false );
          }
        }else{
          s = KTNEFProperty::formatValue( value, false );
        }
        return upper ? s.toUpper() : s;
      }
    }
  }
  return fallback;
}


QMap<int,KTNEFProperty*>& KTNEFPropertySet::properties()
{
	return properties_;
}

const QMap<int,KTNEFProperty*>& KTNEFPropertySet::properties() const
{
	return properties_;
}

QVariant KTNEFPropertySet::property( int key ) const
{
	QMap<int,KTNEFProperty*>::ConstIterator it = properties_.find( key );
	if ( it == properties_.end() )
		return QVariant();
	else
		return ( *it )->value();
}

void KTNEFPropertySet::clear( bool deleteAll )
{
	if ( deleteAll )
	{
		for ( QMap<int,KTNEFProperty*>::ConstIterator it=properties_.begin(); it!=properties_.end(); ++it )
			delete ( *it );
		for ( QMap<int,KTNEFProperty*>::ConstIterator it=attributes_.begin(); it!=attributes_.end(); ++it )
			delete ( *it );
	}
	properties_.clear();
	attributes_.clear();
}

void KTNEFPropertySet::addAttribute( int key, int type, const QVariant& value, bool overwrite )
{
	QMap<int,KTNEFProperty*>::ConstIterator it = attributes_.find( key );
	if ( it != attributes_.end() )
	{
		if ( overwrite )
			delete ( *it );
		else
			return;
	}
	KTNEFProperty *p = new KTNEFProperty( key, type, value, QVariant() );
	attributes_[ p->key() ] = p;
}

QMap<int,KTNEFProperty*>& KTNEFPropertySet::attributes()
{
	return attributes_;
}

const QMap<int,KTNEFProperty*>& KTNEFPropertySet::attributes() const
{
	return attributes_;
}

QVariant KTNEFPropertySet::attribute( int key ) const
{
	QMap<int,KTNEFProperty*>::ConstIterator it = attributes_.find( key );
	if ( it == attributes_.end() )
		return QVariant();
	else
		return ( *it )->value();
}


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
#include <kdebug.h>

KTNEFPropertySet::KTNEFPropertySet()
{
}

KTNEFPropertySet::~KTNEFPropertySet()
{
	clear( true );
}

void KTNEFPropertySet::addProperty( int key, int type, const TQVariant& value, const TQVariant& name, bool overwrite )
{
	TQMap<int,KTNEFProperty*>::ConstIterator it = properties_.find( key );
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


TQString KTNEFPropertySet::findProp(int key, const TQString& fallback, bool upper)
{
  TQMap<int,KTNEFProperty*>::Iterator it = properties_.find( key );
  if( properties_.end() != it )
    return upper ? KTNEFProperty::formatValue( (*it)->value(), false ).upper()
                 : KTNEFProperty::formatValue( (*it)->value(), false );
  else
    return fallback;
}


TQString KTNEFPropertySet::findNamedProp(const TQString& name, const TQString& fallback, bool upper)
{
  for ( TQMap<int,KTNEFProperty*>::Iterator it = properties_.begin();
        it != properties_.end();
        ++it ){
    if ( (*it)->name().isValid() ){
      TQString s;
      if ( (*it)->name().type() == TQVariant::String )
        s = (*it)->name().asString();
      else
        s = TQString().sprintf( "0X%04X", (*it)->name().asUInt() );
      
      if( s.upper() == name.upper() ){
        TQVariant value = ( *it )->value();
        if( value.type() == TQVariant::List ){
          s = "";
          for ( TQValueList<TQVariant>::ConstIterator lit = value.listBegin();
                lit != value.listEnd();
                ++lit ){
            if( !s.isEmpty() )
              s += ',';
            s += KTNEFProperty::formatValue( *lit, false );
          }
        }else{
          s = KTNEFProperty::formatValue( value, false );
        }
        return upper ? s.upper() : s;
      }
    }
  }
  return fallback;
}


TQMap<int,KTNEFProperty*>& KTNEFPropertySet::properties()
{
	return properties_;
}

const TQMap<int,KTNEFProperty*>& KTNEFPropertySet::properties() const
{
	return properties_;
}

TQVariant KTNEFPropertySet::property( int key ) const
{
	TQMap<int,KTNEFProperty*>::ConstIterator it = properties_.find( key );
	if ( it == properties_.end() )
		return TQVariant();
	else
		return ( *it )->value();
}

void KTNEFPropertySet::clear( bool deleteAll )
{
	if ( deleteAll )
	{
		for ( TQMap<int,KTNEFProperty*>::ConstIterator it=properties_.begin(); it!=properties_.end(); ++it )
			delete ( *it );
		for ( TQMap<int,KTNEFProperty*>::ConstIterator it=attributes_.begin(); it!=attributes_.end(); ++it )
			delete ( *it );
	}
	properties_.clear();
	attributes_.clear();
}

void KTNEFPropertySet::addAttribute( int key, int type, const TQVariant& value, bool overwrite )
{
	TQMap<int,KTNEFProperty*>::ConstIterator it = attributes_.find( key );
	if ( it != attributes_.end() )
	{
		if ( overwrite )
			delete ( *it );
		else
			return;
	}
	KTNEFProperty *p = new KTNEFProperty( key, type, value, TQVariant() );
	attributes_[ p->key() ] = p;
}

TQMap<int,KTNEFProperty*>& KTNEFPropertySet::attributes()
{
	return attributes_;
}

const TQMap<int,KTNEFProperty*>& KTNEFPropertySet::attributes() const
{
	return attributes_;
}

TQVariant KTNEFPropertySet::attribute( int key ) const
{
	TQMap<int,KTNEFProperty*>::ConstIterator it = attributes_.find( key );
	if ( it == attributes_.end() )
		return TQVariant();
	else
		return ( *it )->value();
}


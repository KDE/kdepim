/*
    ktnefproperty.cpp

    Copyright (C) 2002 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "ktnef/ktnefproperty.h"
#include "mapi.h"
#include <qdatetime.h>
#include <ctype.h>

KTNEFProperty::KTNEFProperty()
{
}

KTNEFProperty::KTNEFProperty( int key_, int type_, const QVariant& value_, const QVariant& name_ )
	: _key( key_ ), _type( type_ ), _value( value_ ), _name( name_ )
{
}

KTNEFProperty::KTNEFProperty( const KTNEFProperty& p )
	: _key( p._key ), _type( p._type ), _value( p._value ), _name( p._name )
{
}

QString KTNEFProperty::keyString()
{
	if ( _name.isValid() )
	{
		if ( _name.type() == QVariant::String )
			return _name.asString();
		else
			return mapiNamedTagString( _name.asUInt(), _key );
	}
	else
		return mapiTagString( _key );
}

QString KTNEFProperty::formatValue( const QVariant& value, bool beautify )
{
	if ( value.type() == QVariant::ByteArray )
	{
		// check the first bytes (up to 8) if they are
		// printable characters
		QByteArray arr = value.toByteArray();
		bool printable = true;
		for ( int i=QMIN( arr.size(), 8 )-1; i>=0 && printable; i-- )
			printable = ( isprint( arr[ i ] ) != 0 );
		if ( !printable )
		{
			QString s;
			uint i;
      uint txtCount = beautify ? QMIN( arr.size(), 32 ) : arr.size();
			for ( i=0; i < txtCount; ++i )
			{
				s.append( QString().sprintf( "%02X", ( uchar )arr[ i ] ) );
				if( beautify )
					s.append( " " );
			}
			if ( i < arr.size() )
				s.append( "... (size=" + QString::number( arr.size() ) + ")" );
			return s;
		}
	}
	//else if ( value.type() == QVariant::DateTime )
	//	return value.toDateTime().toString();
	return value.toString();
}

QString KTNEFProperty::valueString()
{
	return formatValue( _value );
}

int KTNEFProperty::key() const
{ return _key; }

int KTNEFProperty::type() const
{ return _type; }

QVariant KTNEFProperty::value() const
{ return _value; }

QVariant KTNEFProperty::name() const
{ return _name; }

bool KTNEFProperty::isVector() const
{ return ( _value.type() == QVariant::List ); }

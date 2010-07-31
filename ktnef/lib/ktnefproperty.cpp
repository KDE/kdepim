/*
    ktnefproperty.cpp

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

#include "ktnef/ktnefproperty.h"
#include "mapi.h"
#include <tqdatetime.h>
#include <ctype.h>

KTNEFProperty::KTNEFProperty()
{
}

KTNEFProperty::KTNEFProperty( int key_, int type_, const TQVariant& value_, const TQVariant& name_ )
	: _key( key_ ), _type( type_ ), _value( value_ ), _name( name_ )
{
}

KTNEFProperty::KTNEFProperty( const KTNEFProperty& p )
	: _key( p._key ), _type( p._type ), _value( p._value ), _name( p._name )
{
}

TQString KTNEFProperty::keyString()
{
	if ( _name.isValid() )
	{
		if ( _name.type() == TQVariant::String )
			return _name.asString();
		else
			return mapiNamedTagString( _name.asUInt(), _key );
	}
	else
		return mapiTagString( _key );
}

TQString KTNEFProperty::formatValue( const TQVariant& value, bool beautify )
{
	if ( value.type() == TQVariant::ByteArray )
	{
		// check the first bytes (up to 8) if they are
		// printable characters
		TQByteArray arr = value.toByteArray();
		bool printable = true;
		for ( int i=QMIN( arr.size(), 8 )-1; i>=0 && printable; i-- )
			printable = ( isprint( arr[ i ] ) != 0 );
		if ( !printable )
		{
			TQString s;
			uint i;
      uint txtCount = beautify ? QMIN( arr.size(), 32 ) : arr.size();
			for ( i=0; i < txtCount; ++i )
			{
				s.append( TQString().sprintf( "%02X", ( uchar )arr[ i ] ) );
				if( beautify )
					s.append( " " );
			}
			if ( i < arr.size() )
				s.append( "... (size=" + TQString::number( arr.size() ) + ")" );
			return s;
		}
	}
	//else if ( value.type() == TQVariant::DateTime )
	//	return value.toDateTime().toString();
	return value.toString();
}

TQString KTNEFProperty::valueString()
{
	return formatValue( _value );
}

int KTNEFProperty::key() const
{ return _key; }

int KTNEFProperty::type() const
{ return _type; }

TQVariant KTNEFProperty::value() const
{ return _value; }

TQVariant KTNEFProperty::name() const
{ return _name; }

bool KTNEFProperty::isVector() const
{ return ( _value.type() == TQVariant::List ); }

/*
    ktnefproperty.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef KTNEFPROPERTY_H
#define KTNEFPROPERTY_H

#include <qvariant.h>
#include <qstring.h>
#include <kdepimmacros.h>

class KDE_EXPORT KTNEFProperty
{
public:
	enum MAPIType
	{
		UInt16  = 0x0002,
		ULong   = 0x0003,
		Float   = 0x0004,
		Double  = 0x0005,
		Boolean = 0x000B,
		Object  = 0x000D,
		Time    = 0x0040,
		String8 = 0x001E,
		UString = 0x001F,
		Binary  = 0x0102
	};

	KTNEFProperty();
	KTNEFProperty( int key_, int type_, const QVariant& value_, const QVariant& name_ = QVariant() );
	KTNEFProperty( const KTNEFProperty& p );

	QString keyString();
	QString valueString();
	static QString formatValue( const QVariant& v, bool beautify=true );

	int key() const;
	int type() const;
	QVariant value() const;
	QVariant name() const;
	bool isVector() const;

private:
	int _key;
	int _type;
	QVariant _value;
	QVariant _name;
};

#endif /* KTNEFPROPERTY_H */

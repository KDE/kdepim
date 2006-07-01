/*
    ktnefpropertyset.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#ifndef KTNEFPROPERTYSET_H
#define KTNEFPROPERTYSET_H

#include <QMap>
#include <QVariant>
#include <kdepimmacros.h>

class KTNEFProperty;

class KDE_EXPORT KTNEFPropertySet
{
public:
	KTNEFPropertySet();
	~KTNEFPropertySet();

	/* MAPI properties interface */
	void addProperty( int key, int type, const QVariant& value, const QVariant& name = QVariant(), bool overwrite = false );
	QString findProp(     int key,             const QString& fallback=QString(), bool convertToUpper=false);
	QString findNamedProp(const QString& name, const QString& fallback=QString(), bool convertToUpper=false);
	QMap<int,KTNEFProperty*>& properties();
	const QMap<int,KTNEFProperty*>& properties() const;
	QVariant property( int key ) const;

	/* TNEF attributes interface */
	void addAttribute( int key, int type, const QVariant& value, bool overwrite = false );
	QMap<int,KTNEFProperty*>& attributes();
	const QMap<int,KTNEFProperty*>& attributes() const;
	QVariant attribute( int key ) const;

	void clear( bool deleteAll = false );

private:
	QMap<int,KTNEFProperty*> properties_;  /* used to store MAPI properties */
	QMap<int,KTNEFProperty*> attributes_;  /* used to store TNEF attributes */
};

#endif /* KTNEFPROPERTYSET_H */

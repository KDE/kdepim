/*
    ktnefpropertyset.h

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

#ifndef KTNEFPROPERTYSET_H
#define KTNEFPROPERTYSET_H

#include <qmap.h>
#include <qvariant.h>
#include <kdepimmacros.h>

#include <kdemacros.h>

class KTNEFProperty;

class KDE_EXPORT KTNEFPropertySet
{
public:
	KTNEFPropertySet();
	~KTNEFPropertySet();

	/* MAPI properties interface */
	void addProperty( int key, int type, const QVariant& value, const QVariant& name = QVariant(), bool overwrite = false );
	QString findProp(     int key,             const QString& fallback=QString::null, bool convertToUpper=false);
	QString findNamedProp(const QString& name, const QString& fallback=QString::null, bool convertToUpper=false);
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

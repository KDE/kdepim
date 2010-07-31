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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFPROPERTYSET_H
#define KTNEFPROPERTYSET_H

#include <tqmap.h>
#include <tqvariant.h>
#include <kdepimmacros.h>

class KTNEFProperty;

class KDE_EXPORT KTNEFPropertySet
{
public:
	KTNEFPropertySet();
	~KTNEFPropertySet();

	/* MAPI properties interface */
	void addProperty( int key, int type, const TQVariant& value, const TQVariant& name = TQVariant(), bool overwrite = false );
	TQString findProp(     int key,             const TQString& fallback=TQString::null, bool convertToUpper=false);
	TQString findNamedProp(const TQString& name, const TQString& fallback=TQString::null, bool convertToUpper=false);
	TQMap<int,KTNEFProperty*>& properties();
	const TQMap<int,KTNEFProperty*>& properties() const;
	TQVariant property( int key ) const;

	/* TNEF attributes interface */
	void addAttribute( int key, int type, const TQVariant& value, bool overwrite = false );
	TQMap<int,KTNEFProperty*>& attributes();
	const TQMap<int,KTNEFProperty*>& attributes() const;
	TQVariant attribute( int key ) const;

	void clear( bool deleteAll = false );

private:
	TQMap<int,KTNEFProperty*> properties_;  /* used to store MAPI properties */
	TQMap<int,KTNEFProperty*> attributes_;  /* used to store TNEF attributes */
};

#endif /* KTNEFPROPERTYSET_H */

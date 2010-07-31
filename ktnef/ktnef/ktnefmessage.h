/*
    ktnefmessage.h

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

#ifndef KTNEFMESSAGE_H
#define KTNEFMESSAGE_H

#include <ktnef/ktnefpropertyset.h>
#include <tqptrlist.h>
#include <kdepimmacros.h>

class KTNEFAttach;

class KDE_EXPORT KTNEFMessage : public KTNEFPropertySet
{
public:
	KTNEFMessage();
	~KTNEFMessage();

	const TQPtrList<KTNEFAttach>& attachmentList() const;
	KTNEFAttach* attachment( const TQString& filename ) const;
	void addAttachment( KTNEFAttach* attach );
	void clearAttachments();
	TQString rtfString();

private:
	class MessagePrivate;
	MessagePrivate *d;
};

#endif /* KTNEFMESSAGE_H */

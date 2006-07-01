/*
    ktnefmessage.h

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

#ifndef KTNEFMESSAGE_H
#define KTNEFMESSAGE_H

#include <ktnef/ktnefpropertyset.h>
#include <QList>
#include <kdepimmacros.h>

class KTNEFAttach;

class KDE_EXPORT KTNEFMessage : public KTNEFPropertySet
{
public:
	KTNEFMessage();
	~KTNEFMessage();

	const QList<KTNEFAttach*>& attachmentList() const;
	KTNEFAttach* attachment( const QString& filename ) const;
	void addAttachment( KTNEFAttach* attach );
	void clearAttachments();
	QString rtfString();

private:
	class MessagePrivate;
	MessagePrivate *d;
};

#endif /* KTNEFMESSAGE_H */

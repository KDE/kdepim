/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef LSN_REMOTENOTEMESSAGE_H
#define LSN_REMOTENOTEMESSAGE_H

#include <StickyNotes/global.h>

#include <QVariant>

namespace StickyNotes {

class RemoteNoteMessagePrivate;

class LSN_EXPORT RemoteNoteMessage
{
Q_DECLARE_PRIVATE(RemoteNoteMessage)

public:
	enum Type {mtInvalid, mtGet, mtSet};

public:
	RemoteNoteMessage(void);
	RemoteNoteMessage(Type _type, const QString &_field, const QVariant &_data);
	virtual ~RemoteNoteMessage(void);

	QVariant data(void) const;
	QString  field(void) const;
	bool     isValid(void) const;
	Type     type(void) const;

	void deserialize(const QByteArray &_data);
	QByteArray serialize(void) const;

private:
	RemoteNoteMessagePrivate *d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_REMOTENOTEMESSAGE_H


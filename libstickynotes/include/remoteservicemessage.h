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
 
#ifndef LSN_REMOTESERVICEMESSAGE_H
#define LSN_REMOTESERVICEMESSAGE_H

#include <StickyNotes/global.h>

namespace StickyNotes {

class RemoteServiceMessagePrivate;

class LSN_EXPORT RemoteServiceMessage
{
Q_DECLARE_PRIVATE(RemoteServiceMessage)

public:
	enum Type {mtInvalid, mtCreate, mtDestroy, mtDirect};

public:
	RemoteServiceMessage(void);
	RemoteServiceMessage(Type _type, quint32 _id, const QByteArray &_data);
	virtual ~RemoteServiceMessage(void);

	QByteArray data(void) const;
	Type       type(void) const;
	bool       isValid(void) const;
	quint32    id(void) const;

	void deserialize(const QByteArray &_data);
	QByteArray serialize(void) const;

private:
	RemoteServiceMessagePrivate *d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_REMOTESERVICEMESSAGE_H


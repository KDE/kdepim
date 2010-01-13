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
 
#include "../include/remoteservicemessage.h"

#include <QBuffer>
#include <QVariant>

using namespace StickyNotes;

namespace StickyNotes {

/* RemoteServiceMessagePrivate */

class RemoteServiceMessagePrivate
{
Q_DECLARE_PUBLIC(RemoteServiceMessage)

public:
	RemoteServiceMessagePrivate(RemoteServiceMessage *_q);

private:
	RemoteServiceMessage *q_ptr;
	QByteArray data;
	quint32    id;
	RemoteServiceMessage::Type type;
	bool valid;
};

} // namespace StickyNotes

RemoteServiceMessagePrivate::RemoteServiceMessagePrivate(RemoteServiceMessage *_q)
: q_ptr(_q), id(0), type(RemoteServiceMessage::mtInvalid), valid(false)
{
	data.clear();
}

/* RemoteServiceMessage */

RemoteServiceMessage::RemoteServiceMessage(void)
: d_ptr(new RemoteServiceMessagePrivate(this))
{
}

RemoteServiceMessage::RemoteServiceMessage(Type _type, quint32 _id,
    const QByteArray &_data)
: d_ptr(new RemoteServiceMessagePrivate(this))
{
	Q_D(RemoteServiceMessage);

	if (mtInvalid != (d->type = _type)) {
		d->data  = _data;
		d->id    = _id;
		d->valid = true;
	}
}

RemoteServiceMessage::~RemoteServiceMessage(void)
{
	delete d_ptr;
}

QByteArray
RemoteServiceMessage::data(void) const
{
	return (d_func()->data);
}

RemoteServiceMessage::Type
RemoteServiceMessage::type(void) const
{
	return (d_func()->type);
}

bool
RemoteServiceMessage::isValid(void) const
{
	return (d_func()->valid);
}

quint32
RemoteServiceMessage::id(void) const
{
	return (d_func()->id);
}

void
RemoteServiceMessage::deserialize(const QByteArray &_data)
{
	Q_D(RemoteServiceMessage);
	QBuffer buffer;
	quint32 dataSize;
	char   *tmpData;

	if (9 > _data.size())
		return;

	d->data.clear();
	d->id    = 0;
	d->type  = mtInvalid;
	d->valid = false;

	buffer.setData(_data);
	buffer.open(QIODevice::ReadOnly);

	buffer.read(reinterpret_cast<char *>(&d->type), 1);

	if (mtInvalid == d->type)
		return;

	buffer.read(reinterpret_cast<char *>(&d->id), 4);

	if (0 == d->id)
		return;

	d->valid = true;

	buffer.read(reinterpret_cast<char *>(&dataSize), 4);

	if (0 == dataSize)
		return;

	tmpData = new char[dataSize];
	{
		buffer.read(tmpData, dataSize);
		d->data.append(tmpData, dataSize);
	}
	delete [] tmpData;

	buffer.close();
}

QByteArray
RemoteServiceMessage::serialize(void) const
{
	const RemoteServiceMessagePrivate *d = d_func();
	QByteArray  badata;
	QBuffer	    buffer(&badata);
	quint32     dataSize;

	dataSize = d->data.size();

	buffer.open(QIODevice::WriteOnly);
	buffer.write(reinterpret_cast<const char *>(&d->type), 1);
	buffer.write(reinterpret_cast<const char *>(&d->id), 4);
	buffer.write(reinterpret_cast<const char *>(&dataSize), 4);
	buffer.write(d->data.data(), dataSize);
	buffer.close();

	return (badata);
}


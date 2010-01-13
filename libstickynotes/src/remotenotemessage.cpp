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
 
#include "../include/remotenotemessage.h"

#include <QBuffer>

using namespace StickyNotes;

namespace StickyNotes {

/* RemoteNoteMessagePrivate */

class RemoteNoteMessagePrivate
{
Q_DECLARE_PUBLIC(RemoteNoteMessage)

public:
	RemoteNoteMessagePrivate(RemoteNoteMessage *_q);

private:
	RemoteNoteMessage *q_ptr;
	QVariant data;
	QString  field;
	RemoteNoteMessage::Type type;
	bool valid;
};

} // namespace StickyNotes

RemoteNoteMessagePrivate::RemoteNoteMessagePrivate(RemoteNoteMessage *_q)
: q_ptr(_q), type(RemoteNoteMessage::mtInvalid), valid(false)
{
	data.clear();
	field.clear();
}

/* RemoteNoteMessage */

RemoteNoteMessage::RemoteNoteMessage(void)
: d_ptr(new RemoteNoteMessagePrivate(this))
{
}

RemoteNoteMessage::RemoteNoteMessage(Type _type, const QString &_field,
    const QVariant &_data)
: d_ptr(new RemoteNoteMessagePrivate(this))
{
	Q_D(RemoteNoteMessage);

	if (mtInvalid != (d->type = _type)) {
		d->data  = _data;
		d->field = _field;
		d->valid = true;
	}
}

RemoteNoteMessage::~RemoteNoteMessage(void)
{
	delete d_ptr;
}

QVariant
RemoteNoteMessage::data(void) const
{
	return (d_func()->data);
}

QString
RemoteNoteMessage::field(void) const
{
	return (d_func()->field);
}

bool
RemoteNoteMessage::isValid(void) const
{
	return (d_func()->valid);
}

RemoteNoteMessage::Type
RemoteNoteMessage::type(void) const
{
	return (d_func()->type);
}

void
RemoteNoteMessage::deserialize(const QByteArray &_data)
{
	Q_D(RemoteNoteMessage);
	QBuffer buffer;
	quint16 dataSize;
	char   *tmpData;

	if (5 > _data.size())
		return;

	QDataStream dstream(_data);

	d->data.clear();
	d->field.clear();
	d->type  = mtInvalid;
	d->valid = false;

	buffer.setData(_data);
	buffer.open(QIODevice::ReadOnly);

    	buffer.read(reinterpret_cast<char *>(&d->type), 1);

	if (mtInvalid == d->type)
		return;

	d->valid = true;

	buffer.read(reinterpret_cast<char *>(&dataSize), 2);

	if (0 == dataSize)
		return;

	tmpData = new char[dataSize];
	{
		buffer.read(tmpData, dataSize);
		d->field = QString::fromUtf8(tmpData, dataSize);
	}
	delete [] tmpData;

	buffer.read(reinterpret_cast<char *>(&dataSize), 2);

	if (0 == dataSize)
		return;

	tmpData = new char[dataSize];
	{
		buffer.read(tmpData, dataSize);
		d->data = QString::fromUtf8(tmpData, dataSize);
	}
	delete [] tmpData;
}

QByteArray
RemoteNoteMessage::serialize(void) const
{
	const RemoteNoteMessagePrivate *d = d_func();
	QByteArray badata;
	QBuffer buffer(&badata);
	QString data(d->data.toString());
	quint16 dataSize;
	quint16 fieldSize;

	dataSize  = data.size();
	fieldSize = d->field.size();

	buffer.open(QIODevice::WriteOnly);
	buffer.write(reinterpret_cast<const char *>(&d->type), 1);
	buffer.write(reinterpret_cast<const char *>(&fieldSize), 2);
	buffer.write(d->field.toUtf8().data(), fieldSize);
	buffer.write(reinterpret_cast<const char *>(&dataSize), 2);
	buffer.write(data.toUtf8().data(), dataSize);
	buffer.close();

	return (badata);
}


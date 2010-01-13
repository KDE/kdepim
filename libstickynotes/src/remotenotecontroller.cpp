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
 
#include "../include/remotenotecontroller.h"

#include <QIODevice>

#include "../include/remotenoteitem.h"
#include "../include/remotenotemessage.h"
#include "../include/remoteservicemessage.h"

using namespace StickyNotes;

namespace StickyNotes {

/* RemoteNoteControllerPrivate */

class RemoteNoteControllerPrivate
{
Q_DECLARE_PUBLIC(RemoteNoteController)

public:
	RemoteNoteControllerPrivate(RemoteNoteController *_q);

private:
	RemoteNoteController *q_ptr;
	QByteArray buffer;
	QIODevice *device;
	QMap<quint32, RemoteNoteItem *> items;
	quint32 nextid;
};

} // namespace StickyNotes

RemoteNoteControllerPrivate::RemoteNoteControllerPrivate(RemoteNoteController *_q)
: q_ptr(_q), device(0), nextid(0)
{
	items.clear();
}

/* RemoteNoteController */

const char * const RemoteNoteController::EOT = "\xFF\x04\xFF";

RemoteNoteController::RemoteNoteController(QIODevice &_device) 
: QObject(&_device), d_ptr(new RemoteNoteControllerPrivate(this))
{
	d_func()->device = &_device;
}

RemoteNoteController::~RemoteNoteController(void)
{
	Q_D(RemoteNoteController);

	foreach(RemoteNoteItem *item, d->items.values())
		delete item;

	delete d_ptr;
}

QIODevice &
RemoteNoteController::device(void) const
{
	return (*d_func()->device);
}

void
RemoteNoteController::startListening(void)
{
	Q_D(RemoteNoteController);

	connect(d->device, SIGNAL(readChannelFinished(void)),
	    this, SLOT(on_device_finished(void)));

	connect(d->device, SIGNAL(readyRead(void)),
	    this, SLOT(on_device_readyRead(void)));

	// Check if anything has arrived before we started listening.
	on_device_readyRead();
}

void
RemoteNoteController::stopListening(void)
{
	Q_D(RemoteNoteController);

	disconnect(d->device, SIGNAL(readChannelFinished(void)),
	    this, SLOT(on_device_finished(void)));

	disconnect(d->device, SIGNAL(readyRead(void)),
	    this, SLOT(on_device_readyRead(void)));

	// Check if anything arrived before we stopped listening.
	on_device_readyRead();

}

RemoteNoteItem *
RemoteNoteController::createItem(void)
{
	Q_D(RemoteNoteController);
	quint32 cid;
	RemoteNoteItem *item;

	cid = ++d->nextid;

	item = new RemoteNoteItem(*this, cid, true);
	d->items.insert(cid, item);

	sendMessage(RemoteServiceMessage::mtCreate, cid, RemoteNoteMessage());

	return (item);
}

void
RemoteNoteController::handleMessage(RemoteServiceMessage &_message)
{
	Q_D(RemoteNoteController);
	RemoteNoteItem *item;

	if (!_message.isValid())
		return;

	switch (_message.type())
	{
		case RemoteServiceMessage::mtCreate: {
			item = new RemoteNoteItem(*this, _message.id(), false);

			if (d->items.contains(_message.id()))
			    d->items[_message.id()] = item;
			else
			    d->items.insert(_message.id(), item);

			emit createdItem(*item);
		}
		break;

		case RemoteServiceMessage::mtDestroy: {
			if (!d->items.contains(_message.id()))
				return;

			item = d->items.value(_message.id());
    
			emit destroyingItem(*item);

			delete item;
		}
		break;

		case RemoteServiceMessage::mtDirect: {
			RemoteNoteMessage msg;

			if (!d->items.contains(_message.id()))
				return;

			item = d->items.value(_message.id());

			msg.deserialize(_message.data());

			if (!msg.isValid())
				return;

			item->handleMessage(msg);
		}
		break;

		default: return; break;
	}
}

void
RemoteNoteController::removeItem(quint32 _id)
{
	d_func()->items.remove(_id);
}

void
RemoteNoteController::sendMessage(RemoteServiceMessage::Type _type,
    quint32 _id, const RemoteNoteMessage &_message) const
{
	RemoteServiceMessage msg(_type, _id, _message.serialize());
	QByteArray buffer(msg.serialize());

	buffer.append(EOT);

	device().write(buffer);
}

void
RemoteNoteController::on_device_finished(void)
{
	Q_D(RemoteNoteController);

	if (!d->device->atEnd())
		on_device_readyRead();
}

void
RemoteNoteController::on_device_readyRead(void)
{
	Q_D(RemoteNoteController);
	d->buffer.append(d->device->readAll());
	int EOTpos;

	// find EOT character
	while (0 <= (EOTpos = d->buffer.indexOf(EOT))) {
		RemoteServiceMessage msg;
		QByteArray csm(d->buffer.mid(0, EOTpos));
		
		d->buffer.remove(0, EOTpos + (sizeof(EOT) - 1));

		msg.deserialize(csm);

		if (!msg.isValid())
			continue;

		handleMessage(msg);
	}
}

#include "../include/remotenotecontroller.moc"


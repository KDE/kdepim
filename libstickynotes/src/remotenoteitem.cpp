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
 
#include "../include/remotenoteitem.h"

#include "../include/remotenotecontroller.h"

using namespace StickyNotes;

namespace StickyNotes {

/* RemoteNoteItemPrivate */

class RemoteNoteItemPrivate
{
Q_DECLARE_PUBLIC(RemoteNoteItem)

public:
	RemoteNoteItemPrivate(RemoteNoteItem *_q);

private:
	RemoteNoteItem *q_ptr;
	RemoteNoteController *controller;
	quint32 id;
	bool master;
};

} // namespace StickyNotes

RemoteNoteItemPrivate::RemoteNoteItemPrivate(RemoteNoteItem *_q)
: q_ptr(_q), controller(0), id(0), master(false)
{
}

/* RemoteNoteItem */

RemoteNoteItem::RemoteNoteItem(RemoteNoteController &_controller,
    quint32 _id, bool _master)
: BaseNoteItem(0), d_ptr(new RemoteNoteItemPrivate(this))
{
	Q_D(RemoteNoteItem);

	d->controller = &_controller;
	d->id = _id;
	d->master = _master;
}

RemoteNoteItem::~RemoteNoteItem(void)
{
	Q_D(RemoteNoteItem);

	d->controller->removeItem(d->id);

	delete d_ptr;
}

RemoteNoteController &
RemoteNoteItem::controller(void) const
{
	return (*d_func()->controller);
}

bool
RemoteNoteItem::setParent(BaseNoteItem *_parent)
{
	bool changed;

	changed = BaseNoteItem::setParent(_parent);

	if (changed && _parent) {
		if (d_func()->master) {
			sendMessage(RemoteNoteMessage::mtSet,
			    "subject", subject());
			sendMessage(RemoteNoteMessage::mtSet,
			    "content", content());
			foreach (QString name, attributeNames())
				sendMessage(RemoteNoteMessage::mtSet,
				    name, attribute(name));
		} else {
			QVariant value;
			sendMessage(RemoteNoteMessage::mtGet,
			    "subject", value);
			sendMessage(RemoteNoteMessage::mtGet,
			    "content", value);
			foreach (QString name, attributeNames()) 
				sendMessage(RemoteNoteMessage::mtGet,
				    name,  value);
		}
	}

	return (changed);
}

void
RemoteNoteItem::handleMessage(const RemoteNoteMessage &_message)
{
	switch (_message.type())
	{
		case RemoteNoteMessage::mtGet:
			if (0 == _message.field().compare("subject"))
				sendMessage(RemoteNoteMessage::mtSet,
				    "subject", subject());
			else if (0 == _message.field().compare("content"))
				sendMessage(RemoteNoteMessage::mtSet,
				    "content", content());
			else
				sendMessage(RemoteNoteMessage::mtSet,
				    _message.field(), subject());
			break;

		case RemoteNoteMessage::mtSet:
			if (0 == _message.field().compare("subject"))
				applySubject(this, _message.data().toString());
			else if (0 == _message.field().compare("content"))
				applyContent(this, _message.data().toString());
			else
				applyAttribute(this, _message.field(), _message.data());
			break;

		default: return; break;
	}
}

void
RemoteNoteItem::sendMessage(RemoteNoteMessage::Type _type, const QString &_field,
	const QVariant &_data) const
{
	RemoteNoteMessage msg(_type, _field, _data);
	d_func()->controller->sendMessage(RemoteServiceMessage::mtDirect, d_func()->id, msg);
}

bool
RemoteNoteItem::applyAttribute(BaseNoteItem * const _sender, const QString &_name,
    const QVariant &_value)
{
	bool changed;

	changed = BaseNoteItem::applyAttribute(_sender, _name, _value);

	if (changed && this != _sender)
		sendMessage(RemoteNoteMessage::mtSet, _name, _value);

	return (changed);
}

bool
RemoteNoteItem::applyContent(BaseNoteItem * const _sender, const QString &_content)
{
	bool changed;

	changed = BaseNoteItem::applyContent(_sender, _content);

	if (changed && this != _sender)
		sendMessage(RemoteNoteMessage::mtSet, "content", _content);

	return (changed);
}

bool
RemoteNoteItem::applySubject(BaseNoteItem * const _sender, const QString &_subject)
{
	bool changed;

	changed = BaseNoteItem::applySubject(_sender, _subject);

	if (changed && this != _sender)
		sendMessage(RemoteNoteMessage::mtSet, "subject", _subject);

	return (changed);
}


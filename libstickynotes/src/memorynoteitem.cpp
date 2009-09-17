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
 
#include "../include/memorynoteitem.h"

using namespace StickyNotes;

namespace StickyNotes {

/* MemoryNoteItemPrivate */

class MemoryNoteItemPrivate
{
Q_DECLARE_PUBLIC(MemoryNoteItem)

public:
	MemoryNoteItemPrivate(MemoryNoteItem *_q);

private:
	MemoryNoteItem *q_ptr;
	QMap<QString, QVariant> attributes;
	QString content;
	QString subject;
};

} // namespace StickyNotes

MemoryNoteItemPrivate::MemoryNoteItemPrivate(MemoryNoteItem *_q)
: q_ptr(_q)
{
	attributes.clear();
	subject.clear();
	content.clear();
}

/* MemoryNoteItem */

MemoryNoteItem::MemoryNoteItem(const QString &_subject, const QString &_content)
: BaseNoteItem(0), d_ptr(new MemoryNoteItemPrivate(this))
{
	init(_subject, _content);
}

MemoryNoteItem::~MemoryNoteItem(void)
{
	delete d_ptr;
}

QVariant
MemoryNoteItem::attribute(const QString &_name) const
{
	return (d_func()->attributes[_name]);
}

QList<QString>
MemoryNoteItem::attributeNames(void) const
{
	return (d_func()->attributes.keys());
}

QString
MemoryNoteItem::content(void) const
{
	return (d_func()->content);
}

QString
MemoryNoteItem::subject(void) const
{
	return (d_func()->subject);
}

void
MemoryNoteItem::init(const QString &_subject, const QString &_content)
{
	applySubject(this, _subject);
	applyContent(this, _content);
}

bool
MemoryNoteItem::applyAttribute(BaseNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	Q_D(MemoryNoteItem);

	if (d->attributes[_name] == _value)
		return false;

	d->attributes[_name] = _value;

	pushAttribute(_sender, _name, _value);

	return (true);
}

bool
MemoryNoteItem::applyContent(BaseNoteItem * const _sender,
    const QString &_content)
{
	Q_D(MemoryNoteItem);

	if (0 == d->content.compare(_content))
		return (false);

	d->content = _content;

	pushContent(_sender, _content);

	return (true);
}

bool
MemoryNoteItem::applySubject(BaseNoteItem * const _sender,
    const QString &_subject)
{
	Q_D(MemoryNoteItem);

	if (0 == d->subject.compare(_subject))
		return (false);

	d->subject = _subject;

	pushSubject(_sender, _subject);

	return (true);
}


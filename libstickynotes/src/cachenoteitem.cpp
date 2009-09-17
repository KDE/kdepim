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
 
#include "../include/cachenoteitem.h"

using namespace StickyNotes;

namespace StickyNotes {

/* CacheNoteItemPrivate */

class CacheNoteItemPrivate
{
Q_DECLARE_PUBLIC(CacheNoteItem)

public:
	CacheNoteItemPrivate(CacheNoteItem *_q);

private:
	CacheNoteItem *q_ptr;
	QMap<QString, QVariant> attributes;
	QString content;
	QString subject;
};

} // namespace StickyNotes

CacheNoteItemPrivate::CacheNoteItemPrivate(CacheNoteItem *_q)
: q_ptr(_q)
{
	attributes.clear();
	subject.clear();
	content.clear();
}

/* CacheNoteItem */

CacheNoteItem::CacheNoteItem(BaseNoteItem *_parent)
: BaseNoteItem(0), d_ptr(new CacheNoteItemPrivate(this))
{
	setParent(_parent);
}

CacheNoteItem::~CacheNoteItem(void)
{
	delete d_ptr;
}

QVariant
CacheNoteItem::attribute(const QString &_name) const
{
	return (d_func()->attributes[_name]);
}

QList<QString>
CacheNoteItem::attributeNames(void) const
{
	return (d_func()->attributes.keys());
}

QString
CacheNoteItem::content(void) const
{
	return (d_func()->content);
}

QString
CacheNoteItem::subject(void) const
{
	return (d_func()->subject);
}

bool
CacheNoteItem::setParent(BaseNoteItem *_parent)
{
	Q_D(CacheNoteItem);

	if (BaseNoteItem::setParent(_parent)) {
		if (_parent) {
			d->content = _parent->content();
			d->subject = _parent->subject();

			foreach( QString attribute,
			    _parent->attributeNames())
				d->attributes[attribute] =
				    _parent->attribute(attribute);
		}

		return (true);
	}

	return (false);
}

bool
CacheNoteItem::applyAttribute(BaseNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	Q_D(CacheNoteItem);

	if (d->attributes[_name] == _value)
		return false;

	if (!BaseNoteItem::applyAttribute(this, _name, _value))
		return (false);

	d->attributes[_name] = _value;

	return (true);
}

bool
CacheNoteItem::applyContent(BaseNoteItem * const _sender,
    const QString &_content)
{
	Q_D(CacheNoteItem);

	if (0 == d->content.compare(_content))
		return (false);

	if (!BaseNoteItem::applyContent(this, _content))
		return (false);

	d->content = _content;

	return (true);
}

bool
CacheNoteItem::applySubject(BaseNoteItem * const _sender,
    const QString &_subject)
{
	Q_D(CacheNoteItem);

	if (0 == d->subject.compare(_subject))
		return (false);

	if (!BaseNoteItem::applySubject(this, _subject))
		return (false);

	d->subject = _subject;

	return (true);
}


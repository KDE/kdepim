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
 
#include "../include/hooknoteitem.h"

using namespace StickyNotes;

/* HookNoteItem */

HookNoteItem::HookNoteItem(BaseNoteItem *_parent)
: BaseNoteItem(0)
{
	setParent(_parent);
}

HookNoteItem::~HookNoteItem(void)
{
}

bool
HookNoteItem::setParent(BaseNoteItem *_parent)
{
	if (BaseNoteItem::setParent(_parent)) {
		if (_parent)
			emit bound();
		else
			emit unbound();

		return (true);
	}

	return (false);
}

bool
HookNoteItem::applyAttribute(BaseNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	if (!BaseNoteItem::applyAttribute(_sender, _name, _value))
		return (false);

	emit appliedAttribute(_name, _value);

	return (true);
}

bool
HookNoteItem::applyContent(BaseNoteItem * const _sender,
    const QString &_content)
{
	if (!BaseNoteItem::applyContent(_sender, _content))
		return (false);

	emit appliedContent(_content);

	return (true);
}

bool
HookNoteItem::applySubject(BaseNoteItem * const _sender,
    const QString &_subject)
{
	if (!BaseNoteItem::applySubject(_sender, _subject))
		return (false);

	emit appliedSubject(_subject);

	return (true);
}

#include "../include/hooknoteitem.moc"


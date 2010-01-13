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
 
#include "../include/basenoteitem.h"

using namespace StickyNotes;

namespace StickyNotes {

/* BaseNoteItemPrivate */

class BaseNoteItemPrivate
{
Q_DECLARE_PUBLIC(BaseNoteItem)

public:
	BaseNoteItemPrivate(BaseNoteItem *_q);

private:
	BaseNoteItem *q_ptr;
	QList<AbstractNoteItem *> children;
	BaseNoteItem *parent;
};

} // namespace StickyNotes

BaseNoteItemPrivate::BaseNoteItemPrivate(BaseNoteItem *_q)
: q_ptr(_q), parent(0)
{
}

/* BaseNoteItem */

BaseNoteItem::BaseNoteItem(BaseNoteItem *_parent)
: d_ptr(new BaseNoteItemPrivate(this))
{
	setParent(_parent);
}

BaseNoteItem::~BaseNoteItem(void)
{
	Q_D(BaseNoteItem);

	// delete all children that are of BaseNoteItem descent.
	for (int i = (d->children.count() - 1); i >= 0; --i) 
		delete dynamic_cast<BaseNoteItem *>(d->children.at(i));

	// unbind from parent (if any).
	setParent(0);

	delete d_ptr;
}

const QList<AbstractNoteItem *> &
BaseNoteItem::children(void) const
{
	return (d_func()->children);
}

BaseNoteItem *
BaseNoteItem::parent(void) const
{
	return (d_func()->parent);
}

QVariant
BaseNoteItem::attribute(const QString &_name) const
{
	const BaseNoteItemPrivate *d = d_func();

	if (d->parent)
		return (d->parent->attribute(_name));

	return (QVariant());
}

QList<QString>
BaseNoteItem::attributeNames(void) const
{
	const BaseNoteItemPrivate *d = d_func();

	if (d->parent)
		return (d->parent->attributeNames());

	return (QList<QString>());
}

QString
BaseNoteItem::content(void) const
{
	const BaseNoteItemPrivate *d = d_func();

	if (d->parent)
		return (d->parent->content());

	return(QString());
}

void
BaseNoteItem::setAttribute(const QString &_name, const QVariant &_value)
{
	applyAttribute(0, _name, _value);
}

void
BaseNoteItem::setContent(const QString &_content)
{
	applyContent(0, _content);
}

bool
BaseNoteItem::setParent(BaseNoteItem *_parent)
{
	Q_D(BaseNoteItem);

	if (d->parent == _parent)
		return (false);

	if (d->parent)
		d->parent->unbindChild(this);

	if (!_parent || _parent->bindChild(this)) {
		d->parent = _parent;
		return (true);
	}

	return (false);
}

void
BaseNoteItem::setSubject(const QString &_subject)
{
	applySubject(0, _subject);
}

QString
BaseNoteItem::subject(void) const
{
	const BaseNoteItemPrivate *d = d_func();

	if (d->parent)
		return (d->parent->subject());

	return (QString());
}

void
BaseNoteItem::pushAttribute(AbstractNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	Q_D(BaseNoteItem);

	// push update up to children
	foreach (AbstractNoteItem *child, d->children)
		if (_sender != child) {
			if (BaseNoteItem *bnchild =
			    dynamic_cast<BaseNoteItem *>(child))
				bnchild->applyAttribute(this, _name, _value);
			else
				child->setAttribute(_name, _value);
		}
}

void
BaseNoteItem::pushContent(AbstractNoteItem * const _sender,
    const QString &_content)
{
	Q_D(BaseNoteItem);

	// push update up to children
	foreach (AbstractNoteItem *child, d->children)
		if (_sender != child) {
			if (BaseNoteItem *bnchild =
			    dynamic_cast<BaseNoteItem *>(child))
				bnchild->applyContent(this, _content);
			else
				child->setContent(_content);
		}
}

void
BaseNoteItem::pushSubject(AbstractNoteItem * const _sender,
    const QString &_subject)
{
	Q_D(BaseNoteItem);

	// push update up to children
	foreach (AbstractNoteItem *child, d->children)
		if (_sender != child) {
			if (BaseNoteItem *bnchild =
			    dynamic_cast<BaseNoteItem *>(child))
				bnchild->applySubject(this, _subject);
			else
				child->setSubject(_subject);
		}
}

bool
BaseNoteItem::applyAttribute(BaseNoteItem * const _sender,
    const QString &_name, const QVariant &_value)
{
	Q_D(BaseNoteItem);

	if (!d->parent)
		return (false);

	if (_sender != d->parent
	    && !d->parent->applyAttribute(this, _name, _value))
		return (false);

	pushAttribute(this, _name, _value);

	return (true);
}

bool
BaseNoteItem::applyContent(BaseNoteItem * const _sender,
    const QString &_content)
{
	Q_D(BaseNoteItem);

	if (!d->parent)
		return (false);

	if (_sender != d->parent
	    && !d->parent->applyContent(this, _content))
		return (false);

	pushContent(this, _content);

	return (true);
}

bool
BaseNoteItem::applySubject(BaseNoteItem * const _sender,
    const QString &_subject)
{
	Q_D(BaseNoteItem);

	if (!d->parent)
		return (false);

	if (_sender != d->parent
	    && !d->parent->applySubject(this, _subject))
		return (false);

	pushSubject(this, _subject);

	return (true);
}

bool
BaseNoteItem::bindChild(AbstractNoteItem *_child)
{
	Q_D(BaseNoteItem);

	if (d->children.contains(_child))
		return (false);

	d->children.append(_child);

	// if child is of BaseNoteItem descent, we become its parent.
	if (BaseNoteItem *bnchild = dynamic_cast<BaseNoteItem *>(_child))
		bnchild->setParent(this);

	return (true);
}

bool
BaseNoteItem::unbindChild(AbstractNoteItem *_child)
{
	Q_D(BaseNoteItem);

	if (d->children.removeOne(_child)) {
		// if child is of BaseNoteItem descent, we release it.
		if (BaseNoteItem *bnchild =
		    dynamic_cast<BaseNoteItem *>(_child))
			bnchild->setParent(0);

		return (true);
	}

	return (false);
}


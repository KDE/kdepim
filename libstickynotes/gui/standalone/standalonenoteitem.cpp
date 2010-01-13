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
 
#include "standalonenoteitem.h"

#include "standalonenoteeditor.h"
#include "standalonenotewidget.h"

using namespace StickyNotes;

namespace StickyNotes {

/* StandaloneNoteItemPrivate */

class StandaloneNoteItemPrivate
{
Q_DECLARE_PUBLIC(StandaloneNoteItem)

public:
	StandaloneNoteItemPrivate(StandaloneNoteItem *_q);
	~StandaloneNoteItemPrivate(void);

private:
	StandaloneNoteItem *q_ptr;
	StandaloneNoteEditor *editor;
	StandaloneNoteWidget *widget;
};

} // namespace StickyNotes

StandaloneNoteItemPrivate::StandaloneNoteItemPrivate(StandaloneNoteItem *_q)
: q_ptr(_q), editor(new StandaloneNoteEditor(*_q)),
     widget(new StandaloneNoteWidget(*_q))
{
}

StandaloneNoteItemPrivate::~StandaloneNoteItemPrivate(void)
{
	delete editor;
	delete widget;
}

/* StandaloneNoteItem */

StandaloneNoteItem::StandaloneNoteItem(BaseNoteItem *_parent)
: HookNoteItem(0), d_ptr(new StandaloneNoteItemPrivate(this))
{
	// we need to create d_ptr before we bind to a parent.
	setParent(_parent);
}

StandaloneNoteItem::~StandaloneNoteItem(void)
{
	delete d_ptr;
}

void
StandaloneNoteItem::edit(void)
{
	d_func()->editor->open();
}

#include "include/standalonenoteitem.moc"


/* listCat.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file defines a specialization of KListView that can
** be used to sort some fixed set of object into some fixed
** set of categories.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif

#ifndef _KLOCALE_H_
#include <klocale.h>
#endif


#include "listCat.moc"

static const char *listCat_id =
	"$Id$";

ListCategorizer::ListCategorizer(QWidget *parent, const char *name) :
	KListView(parent,name),
	fStartOpen(false)
{
	setupWidget();
	(void) listCat_id;
}

ListCategorizer::ListCategorizer(const QStringList& i,
	bool startOpen,
	QWidget *parent,
	const char *name) :
	KListView(parent,name),
	fStartOpen(startOpen)
{
	addCategories(i);
}

void ListCategorizer::addCategories(const QStringList& l)
{
	QStringList::ConstIterator i;

	for (i=l.begin(); i!=l.end(); ++i)
	{
		(void) addCategory(*i);
	}
}

QListViewItem *ListCategorizer::addCategory(const QString& name, 
	const QString& desc)
{
	QListViewItem *m = new QListViewItem(this,name,desc);
	m->setSelectable(false);
	m->setOpen(fStartOpen);
	return m;
}
		
void ListCategorizer::setupWidget()
{
	addColumn(i18n("Category"));
	addColumn(i18n("Description"));
	setItemsMovable(false);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropVisualizer(false);
	setRootIsDecorated(true);
}

/* virtual */ bool ListCategorizer::acceptDrag(QDropEvent* event) const
{
	if (!(event->source())) return false;
	QListViewItem *p = itemAt(event->pos());
	if (!p) return false;

	return true;
}

/* virtual */ void ListCategorizer::contentsDropEvent (QDropEvent*e)
{
	cleanDropVisualizer();

	if (!acceptDrag(e)) return;
	e->accept();

	QListViewItem *p = itemAt(e->pos());
	QListViewItem *selection = currentItem();
	if (!p) 
	{
		kdDebug()  << "Drop without a category!"
			<< endl;
		return;
	}

	QListViewItem *category = p->parent();
	if (!category)
	{
		category = p;
	}

	moveItem(selection,category,0L);
}

/* virtual */ void ListCategorizer::startDrag()
{
	QListViewItem *p = currentItem();

	if (!p || !p->parent()) return;

	KListView::startDrag();
}

QStringList ListCategorizer::listSiblings(const QListViewItem *p,
	int column) const
{
	QStringList l;

	while(p)
	{
		l.append(p->text(column));
		p = p->nextSibling();
	}

	return l;
}

QListViewItem *ListCategorizer::findCategory(const QString& category) const
{
	QListViewItem *p = firstChild();

	while (p)
	{
		if (p->text(0) == category) return p;
		p = p->nextSibling();
	}

	return 0L;
}

QListViewItem *ListCategorizer::addItem(const QString& category,
	const QString& name,
	const QString& description)
{
	QListViewItem *p = findCategory(category);

	if (!p) return 0L;

	return new QListViewItem(p,name,description);
}



// $Log$
// Revision 1.3  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.2  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//

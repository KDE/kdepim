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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *listCat_id =
	"$Id$";

#include "options.h"

#include <qpainter.h>
#include <klocale.h>

#include "listCat.moc"


ListCategorizer::ListCategorizer(QWidget * parent,
	const char *name) :
	KListView(parent, name), 
	fStartOpen(false)
{
	FUNCTIONSETUP;
	setupWidget();
	(void) listCat_id;
}

ListCategorizer::ListCategorizer(const QStringList & i,
	bool startOpen,
	QWidget * parent,
	const char *name) :
	KListView(parent, name), 
	fStartOpen(startOpen)
{
	FUNCTIONSETUP;
	addCategories(i);
}

void ListCategorizer::addCategories(const QStringList & l)
{
	FUNCTIONSETUP;
	QStringList::ConstIterator i;

	for (i = l.begin(); i != l.end(); ++i)
	{
		(void) addCategory(*i);
	}
}

QListViewItem *ListCategorizer::addCategory(const QString & name,
	const QString & desc)
{
	FUNCTIONSETUP;
	QListViewItem *m = new QListViewItem(this, name, desc);

	m->setSelectable(false);
	m->setOpen(fStartOpen);
	return m;
}

void ListCategorizer::setupWidget()
{
	FUNCTIONSETUP;
	addColumn(i18n("Category"));
	addColumn(i18n("Description"));
	setItemsMovable(false);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropVisualizer(true);
	setRootIsDecorated(true);
}

/* virtual */ bool ListCategorizer::acceptDrag(QDropEvent * event) const
{
	FUNCTIONSETUP;
	if (!(event->source()))
		return false;
	QListViewItem *p = itemAt(event->pos());

	if (!p)
		return false;

	return true;
}

/* virtual */ void ListCategorizer::contentsDropEvent(QDropEvent * e)
{
	FUNCTIONSETUP;
	cleanDropVisualizer();

	if (!acceptDrag(e))
		return;
	e->accept();

	QListViewItem *p = itemAt(e->pos());
	QListViewItem *selection = currentItem();

	if (!p)
	{
		kdWarning() << "Drop without a category!" << endl;
		return;
	}

	QListViewItem *category = p->parent();

	if (!category)
	{
		category = p;
	}

	moveItem(selection, category, 0L);
}

/* virtual */ void ListCategorizer::startDrag()
{
	FUNCTIONSETUP;
	QListViewItem *p = currentItem();

	if (!p || !p->parent())
		return;

	KListView::startDrag();
}

QStringList ListCategorizer::listSiblings(const QListViewItem * p, int column) const
{
	FUNCTIONSETUP;
	QStringList l;

	while (p)
	{
		l.append(p->text(column));
		p = p->nextSibling();
	}

	return l;
}

QListViewItem *ListCategorizer::findCategory(const QString & category) const
{
	FUNCTIONSETUP;
	QListViewItem *p = firstChild();

	while (p)
	{
		if (p->text(0) == category)
			return p;
		p = p->nextSibling();
	}

	return 0L;
}

QListViewItem *ListCategorizer::addItem(const QString & category,
	const QString & name, const QString & description)
{
	FUNCTIONSETUP;
	QListViewItem *p = findCategory(category);

	if (!p)
		return 0L;

	return new QListViewItem(p, name, description);
}

#define RVPAD	(4)

RichListViewItem::RichListViewItem(QListViewItem *p,
	QString l,
	int c) :
	QListViewItem(p,l)
{
	FUNCTIONSETUP;

	fColumns=c;
	fIsRich = new bool[c];
	fRect = new QRect[c];

	for (int i=0; i<c; i++)
	{
		fIsRich[i]=false;
	}
}

RichListViewItem::~RichListViewItem()
{
	FUNCTIONSETUP;

	delete[] fIsRich;
	delete[] fRect;
}

void RichListViewItem::computeHeight(int c)
{
	FUNCTIONSETUP;

	if (!fIsRich[c]) return;

	QListView *v = listView();
	
	fRect[c] = v->fontMetrics().boundingRect(v->itemMargin()+RVPAD,0+RVPAD,
		v->columnWidth(c)-v->itemMargin()-RVPAD,300,
		AlignLeft | AlignTop | WordBreak,
		text(c));
}


/* virtual */ void RichListViewItem::setup()
{
	FUNCTIONSETUP;

	QListViewItem::setup();

	QListView *v = listView();
	int h = height();

	for (int i=0; i<fColumns; i++)
	{
		computeHeight(i);
		h = QMAX(h,fRect[i].height()+2*RVPAD);
	}

	setHeight(h);
}


/* virtual */ void RichListViewItem::paintCell(QPainter *p,
	const QColorGroup &gc,
	int column,
	int width,
	int alignment)
{
	FUNCTIONSETUP;

	if ((!column) || (!fIsRich[column]))
	{
		QListViewItem::paintCell(p,gc,column,width,alignment);
		return;
	}

	QListView *v = listView();

	p->eraseRect(0,0,width,height());
	p->setBackgroundColor(gc.background());
	p->eraseRect(RVPAD,RVPAD,width-RVPAD,height()-RVPAD);
	p->setPen(gc.text());
	p->drawText(v->itemMargin()+RVPAD,0+RVPAD,
		width-v->itemMargin()-RVPAD,height()-RVPAD,
		AlignTop | AlignLeft | WordBreak,
		text(column),
		-1,
		&fRect[column]);
}





// $Log$
// Revision 1.7  2001/09/30 23:02:13  adridg
// Add support for multi-line comments in conduit configurator and add it to kpilotConfig
//
// Revision 1.6  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.5  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.4  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.3  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.2  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//

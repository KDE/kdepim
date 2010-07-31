/* KPilot
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <tqpainter.h>
#include <klocale.h>

#include "listCat.moc"


ListCategorizer::ListCategorizer(TQWidget * parent,
	const char *name) :
	KListView(parent, name), 
	fStartOpen(false)
{
	FUNCTIONSETUP;
	setupWidget();
}

ListCategorizer::ListCategorizer(const TQStringList & i,
	bool startOpen,
	TQWidget * parent,
	const char *name) :
	KListView(parent, name), 
	fStartOpen(startOpen)
{
	FUNCTIONSETUP;
	addCategories(i);
}

void ListCategorizer::addCategories(const TQStringList & l)
{
	FUNCTIONSETUP;
	TQStringList::ConstIterator i;

	for (i = l.begin(); i != l.end(); ++i)
	{
		(void) addCategory(*i);
	}
}

TQListViewItem *ListCategorizer::addCategory(const TQString & name,
	const TQString & desc)
{
	FUNCTIONSETUP;
	TQListViewItem *m = new TQListViewItem(this, name, desc);

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

/* virtual */ bool ListCategorizer::acceptDrag(TQDropEvent * event) const
{
	FUNCTIONSETUP;
	if (!(event->source()))
		return false;
	TQListViewItem *p = itemAt(event->pos());

	if (!p)
		return false;

	return true;
}

/* virtual */ void ListCategorizer::contentsDropEvent(TQDropEvent * e)
{
	FUNCTIONSETUP;
	cleanDropVisualizer();

	if (!acceptDrag(e))
		return;
	e->accept();

	TQListViewItem *p = itemAt(e->pos());
	TQListViewItem *selection = currentItem();

	if (!p)
	{
		WARNINGKPILOT << "Drop without a category!" << endl;
		return;
	}

	TQListViewItem *category = p->parent();

	if (!category)
	{
		category = p;
	}

	moveItem(selection, category, 0L);
}

/* virtual */ void ListCategorizer::startDrag()
{
	FUNCTIONSETUP;
	TQListViewItem *p = currentItem();

	if (!p || !p->parent())
		return;

	KListView::startDrag();
}

TQStringList ListCategorizer::listSiblings(const TQListViewItem * p, int column) const
{
	FUNCTIONSETUP;
	TQStringList l;

	while (p)
	{
		l.append(p->text(column));
		p = p->nextSibling();
	}

	return l;
}

TQListViewItem *ListCategorizer::findCategory(const TQString & category) const
{
	FUNCTIONSETUP;
	TQListViewItem *p = firstChild();

	while (p)
	{
		if (p->text(0) == category)
			return p;
		p = p->nextSibling();
	}

	return 0L;
}

TQListViewItem *ListCategorizer::addItem(const TQString & category,
	const TQString & name, const TQString & description)
{
	FUNCTIONSETUP;
	TQListViewItem *p = findCategory(category);

	if (!p)
		return 0L;

	return new TQListViewItem(p, name, description);
}

#define RVPAD	(4)

RichListViewItem::RichListViewItem(TQListViewItem *p,
	TQString l,
	int c) :
	TQListViewItem(p,l)
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

	TQListView *v = listView();
	
	fRect[c] = v->fontMetrics().boundingRect(v->itemMargin()+RVPAD,0+RVPAD,
		v->columnWidth(c)-v->itemMargin()-RVPAD,300,
		AlignLeft | AlignTop | WordBreak,
		text(c));
}


/* virtual */ void RichListViewItem::setup()
{
	FUNCTIONSETUP;

	TQListViewItem::setup();

	int h = height();

	for (int i=0; i<fColumns; i++)
	{
		computeHeight(i);
		h = kMax(h,fRect[i].height()+2*RVPAD);
	}

	setHeight(h);
}


/* virtual */ void RichListViewItem::paintCell(TQPainter *p,
	const TQColorGroup &gc,
	int column,
	int width,
	int alignment)
{
	FUNCTIONSETUP;

	if ((!column) || (!fIsRich[column]))
	{
		TQListViewItem::paintCell(p,gc,column,width,alignment);
		return;
	}

	TQListView *v = listView();

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

//Added by qt3to4:
#include <QDropEvent>
/* KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file defines a specialization of K3ListView that can
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

static const char *listCat_id =
	"$Id$";

#include "options.h"

#include <qpainter.h>
#include <klocale.h>

#include "listCat.moc"


ListCategorizer::ListCategorizer(QWidget * parent,
	const char *name) :
	K3ListView(parent, name), 
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
	K3ListView(parent, name), 
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

Q3ListViewItem *ListCategorizer::addCategory(const QString & name,
	const QString & desc)
{
	FUNCTIONSETUP;
	Q3ListViewItem *m = new Q3ListViewItem(this, name, desc);

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
	Q3ListViewItem *p = itemAt(event->pos());

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

	Q3ListViewItem *p = itemAt(e->pos());
	Q3ListViewItem *selection = currentItem();

	if (!p)
	{
		kWarning() << "Drop without a category!" << endl;
		return;
	}

	Q3ListViewItem *category = p->parent();

	if (!category)
	{
		category = p;
	}

	moveItem(selection, category, 0L);
}

/* virtual */ void ListCategorizer::startDrag()
{
	FUNCTIONSETUP;
	Q3ListViewItem *p = currentItem();

	if (!p || !p->parent())
		return;

	K3ListView::startDrag();
}

QStringList ListCategorizer::listSiblings(const Q3ListViewItem * p, int column) const
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

Q3ListViewItem *ListCategorizer::findCategory(const QString & category) const
{
	FUNCTIONSETUP;
	Q3ListViewItem *p = firstChild();

	while (p)
	{
		if (p->text(0) == category)
			return p;
		p = p->nextSibling();
	}

	return 0L;
}

Q3ListViewItem *ListCategorizer::addItem(const QString & category,
	const QString & name, const QString & description)
{
	FUNCTIONSETUP;
	Q3ListViewItem *p = findCategory(category);

	if (!p)
		return 0L;

	return new Q3ListViewItem(p, name, description);
}

#define RVPAD	(4)

RichListViewItem::RichListViewItem(Q3ListViewItem *p,
	QString l,
	int c) :
	Q3ListViewItem(p,l)
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

	Q3ListView *v = listView();
	
	fRect[c] = v->fontMetrics().boundingRect(v->itemMargin()+RVPAD,0+RVPAD,
		v->columnWidth(c)-v->itemMargin()-RVPAD,300,
		Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
		text(c));
}


/* virtual */ void RichListViewItem::setup()
{
	FUNCTIONSETUP;

	Q3ListViewItem::setup();

	int h = height();

	for (int i=0; i<fColumns; i++)
	{
		computeHeight(i);
		h = qMax(h,fRect[i].height()+2*RVPAD);
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
		Q3ListViewItem::paintCell(p,gc,column,width,alignment);
		return;
	}

	Q3ListView *v = listView();

	p->eraseRect(0,0,width,height());
	p->setBackgroundColor(gc.background());
	p->eraseRect(RVPAD,RVPAD,width-RVPAD,height()-RVPAD);
	p->setPen(gc.text());
	p->drawText(v->itemMargin()+RVPAD,0+RVPAD,
		width-v->itemMargin()-RVPAD,height()-RVPAD,
		Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
		text(column),
		-1,
		&fRect[column]);
}

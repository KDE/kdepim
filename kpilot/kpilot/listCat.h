#ifndef LISTCAT_H
#define LISTCAT_H
/* listCat.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a specialization of KListView to allow the user to
** DnD a fixed set of objects into a fixed set of categories
** (categories set at construction time).
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klistview.h>
class QStringList;

/**
 * This Widget extends KListView for a particular purpose:
 * sorting some items into some bins. This can be useful
 * for putting items in an enabled / disabled state, or
 * into categories, or configuring toolbars (putting
 * icons onto toolbars).
 *
 * You can use all of the standard KListView signals and
 * slots. You may in particular want to change the names
 * of the columns, for example:
 * <PRE>
 * ListCategorizer *lc = new ListCategorizer(this,colors);
 * lc->setColumnText(0,i18n("Color"));
 * lc->setColumnText(1,i18n("HTML"));
 * QListViewItem *stdKDE = lc->addCategory(i18n("Standard KDE"));
 * (void) new QListViewItem(stdKDE,i18n("red"),"#FF0000");
 * </PRE>
 * to set sensible column headers for a list of colors
 * and their HTML equivalents (although why you would want
 * to categorize colors is beyond me).
 *
 * @version $Id$
 */

class ListCategorizer : public KListView
{
	Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * This creates a new empty ListCategorizer with
	 * @see startOpen set to false. The parameters
	 * @p parent and @p name are the usual Qt ones.
	 */
	ListCategorizer(QWidget *parent,
		const char *name = 0);
	/**
	 * Constructor.
	 *
	 * This creates a ListCategorizer with the given @p categories
	 * already inserted. In addition, this constructor lets you
	 * specify whether or not @see startOpen is set.
	 */
	ListCategorizer(const QStringList& categories,
		bool startOpen,
		QWidget *parent,
		const char *name = 0);

	/**
	 * Add a list of categories to the ListCategorizer.
	 * All the categories are added without descriptions;
	 * use @see addCategory on a per-category basis for that.
	 */
	void addCategories(const QStringList&);
	/**
	 * Add a category with name @p name and optional
	 * @p description. This can be useful if you want
	 * either a description for the category or want to
	 * refer to this category in the future without
	 * using @see findCategory().
	 *
	 * @return the QListViewItem created for the category
	 */
	QListViewItem *addCategory(const QString& name,
		const QString& description = QString::null);
	/**
	 * Returns the list of names of the categories in
	 * the ListCategorizer.
	 */
	QStringList categories() const
	{
		return listSiblings(firstChild());
	} ;

	/**
	 * Add a single item to the category named @p category,
	 * with name @p name and description set to @p description.
	 * This might be a convenience function, but it's probably
	 * more convenient to just use @see QListViewItem's
	 * constructor. That way you can also hide more data in
	 * the remaining columns.
	 */
	QListViewItem *addItem(const QString& category,
		const QString& name,
		const QString& description = QString::null);
	/**
	 * Returns the list of strings in column @p column under
	 * category @p category. You can do this to get, for example
	 * the names of all the items categorized under a given
	 * category, or, more usefully, set @p column to something
	 * other that 0 (name) or 1 (description) to return the
	 * @see QStringList hidden in the non-visible columns.
	 */
	QStringList items(const QString& category,int column=0) const
	{
		return listSiblings(findCategory(category),column);
	}

	/**
	 * Given a category @see categoryName return the @see QListViewItem
	 * that represents that category. Probably a useless function,
	 * since just remembering the pointer @see addCategory gives
	 * you is faster and uses hardly any memory.
	 */
	QListViewItem *findCategory(const QString& categoryName) const;
	/**
	 * Return the list of strings in column @column of all siblings
	 * of the given item @p p. If you remembered a pointer to a
	 * category, you can use
	 * <PRE>
	 * QStringList l = lc->listSiblings(stdKDE->firstChild(),2);
	 * <PRE>
	 * to get the list of strings in hidden column 2 under
	 * the category you remembered.
	 */
	QStringList listSiblings(const QListViewItem *p,int column=0) const;

	/**
	 * @return whether new categories are inserted in an
	 * open state or not.
	 *
	 * @see setStartOpen
	 */
	bool startOpen() const { return fStartOpen; } ;
	/**
	 * Enable categories being inserted in an open state.
	 * It is disabled by default but may be set from the
	 * constructor.
	 */
	void setStartOpen(bool b) { fStartOpen=b; } ;

protected:
	/**
	 * Reimplemented to prevent categories from being dragged.
	 */
	virtual bool acceptDrag (QDropEvent* event) const;
	/**
	 * Reimplemented to prevent categories from being dragged.
	 */
	virtual void startDrag();
	/**
	 * Reimplemented to prevent categories from being dragged.
	 */
	virtual void contentsDropEvent (QDropEvent*);



private:
	/**
	 * Call several @see KListView functions to set up useful
	 * behavior for this particular class.
	 */
	void setupWidget();

	bool fStartOpen:1;
} ;


class RichListViewItem : public QListViewItem
{
public:
	RichListViewItem(QListViewItem *parent,
		QString,
		int);
	virtual ~RichListViewItem();

	virtual void paintCell(QPainter *,
		const QColorGroup &,
		int column,
		int width,
		int alignment);

	virtual void setup();

	bool isRich(int c) const { return fIsRich[c]; } ;
	void setRich(int c,bool b) { fIsRich[c]=b; } ;

protected:
	void computeHeight(int c);

protected:
	bool *fIsRich;
	QRect *fRect;
	int fColumns;

} ;

#endif

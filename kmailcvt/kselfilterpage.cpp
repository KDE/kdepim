/***************************************************************************
                          kselfilterpage.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <qtextedit.h>
#include <kdebug.h>

#include "kselfilterpage.h"

KSelFilterPage::KSelFilterPage(QWidget *parent, const char *name ) : KSelFilterPageDlg(parent,name) {

	px_introSidebar->setPixmap(locate("data", "kmailcvt/pics/step1.png"));
	kdDebug() << "KSelFilterPage::KSelFilterPage" << endl;
	filterList.setAutoDelete( TRUE );
	connect(_filters, SIGNAL(activated(int)), SLOT(filterSelected(int)));
}

KSelFilterPage::~KSelFilterPage() {
}

void KSelFilterPage::filterSelected(int i)
{
	_desc->setText(filterList.at(i)->info());
}

void KSelFilterPage::addFilter(Filter *f)
{
	filterList.append(f);
	_filters->insertItem(f->name());
	if (_filters->count() == 1) filterSelected(0); // Setup description box with fist filter selected
}

Filter * KSelFilterPage::getSelectedFilter(void)
{
	return filterList.at(_filters->currentItem());
}

void KSelFilterPage::setAuthors(KAboutData& data)
{
	Filter *filterItem;
	for (filterItem = filterList.first(); filterItem; filterItem = filterList.next() ) {
		data.addAuthor(filterItem->author().latin1(), filterItem->name().latin1());
	}
}

#include "kselfilterpage.moc"


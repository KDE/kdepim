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
#include <qtextedit.h>
#include <kdebug.h>
#include <klocale.h>
#include "filters.hxx"

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
	QString info = filterList.at(i)->info();
	QString author = filterList.at(i)->author();
	if(!author.isEmpty())
		info += i18n("<p><i>Written by %1.</i></p>").arg(author);
	_desc->setText(info);
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

#include "kselfilterpage.moc"


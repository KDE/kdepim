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
#include <klocale.h>
#include "filters.hxx"

#include "filter_mbox.hxx"
#include "filter_oe.hxx"
#include "filter_pmail.hxx"
#include "filter_plain.hxx"
#include "filter_evolution.hxx"

#include "kselfilterpage.h"

KSelFilterPage::KSelFilterPage(QWidget *parent, const char *name ) : KSelFilterPageDlg(parent,name) {

	mIntroSidebar->setPixmap(locate("data", "kmailcvt/pics/step1.png"));
	mFilterList.setAutoDelete( TRUE );
	connect(mFilterCombo, SIGNAL(activated(int)), SLOT(filterSelected(int)));

	// Add new filters below. If this annoys you, please rewrite the stuff to use a factory.
        // The former approach was overengineered and only worked around problems in the design
        // For now, we have to live without the warm and fuzzy feeling a refactoring might give. 
        // Patches appreciated. (danimo)

        addFilter(new FilterOE);
        addFilter(new FilterMBox);
        addFilter(new FilterPlain);
        addFilter(new FilterPMail);
        addFilter(new FilterEvolution);
}

KSelFilterPage::~KSelFilterPage() {
}

void KSelFilterPage::filterSelected(int i)
{
	QString info = mFilterList.at(i)->info();
	QString author = mFilterList.at(i)->author();
	if(!author.isEmpty())
		info += i18n("<p><i>Written by %1.</i></p>").arg(author);
	mDesc->setText(info);
}

void KSelFilterPage::addFilter(Filter *f)
{
	mFilterList.append(f);
	mFilterCombo->insertItem(f->name());
	if (mFilterCombo->count() == 1) filterSelected(0); // Setup description box with fist filter selected
}

Filter * KSelFilterPage::getSelectedFilter(void)
{
	return mFilterList.at(mFilterCombo->currentItem());
}

#include "kselfilterpage.moc"


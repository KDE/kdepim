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

// Local includes
#include "kselfilterpage.h"

// Filter includes
#include "filter_mbox.hxx"
#include "filter_oe.hxx"
#include "filter_outlook.hxx"
#include "filter_pmail.hxx"
#include "filter_plain.hxx"
#include "filter_evolution.hxx"
#include "filter_mailapp.hxx"
#include "filter_evolution_v2.hxx"
#include "filter_opera.hxx"
#include "filter_thunderbird.hxx"
#include "filter_kmail_maildir.hxx"
#include "filter_kmail_archive.hxx"
#include "filter_sylpheed.hxx"
#include "filter_thebat.hxx"
#include "filter_lnotes.hxx"

#include "filters.hxx"

// KDE includes
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>

// Qt includes
#include <QCheckBox>
#include <QTimer>

// Akonadi includes
#include <akonadi/collectionrequester.h>
#include <akonadi/control.h>



KSelFilterPage::KSelFilterPage(QWidget *parent ) : KSelFilterPageDlg(parent) {

	mIntroSidebar->setPixmap(KStandardDirs::locate("data", "kmailcvt/pics/step1.png"));
	//mFilterList.setAutoDelete( true );
	connect(mFilterCombo, SIGNAL(activated(int)), SLOT(filterSelected(int)));

	// Add new filters below. If this annoys you, please rewrite the stuff to use a factory.
        // The former approach was overengineered and only worked around problems in the design
        // For now, we have to live without the warm and fuzzy feeling a refactoring might give. 
        // Patches appreciated. (danimo)

        addFilter(new FilterKMailArchive);
        addFilter(new FilterMBox);
        addFilter(new FilterEvolution);
        addFilter(new FilterEvolution_v2);
        addFilter(new FilterKMail_maildir);
        addFilter(new FilterMailApp);
        addFilter(new FilterOpera);
        addFilter(new FilterSylpheed);
        addFilter(new FilterThunderbird);
        addFilter(new FilterTheBat);
        addFilter(new FilterOE);
//        addFilter(new FilterOutlook);
        addFilter(new FilterPMail);
        addFilter(new FilterLNotes);
        addFilter(new FilterPlain);

        // Ensure we return the correct type of Akonadi collection.
        mCollectionRequestor->setMimeTypeFilter( QStringList() << QString( "message/rfc822" ) );
        mCollectionRequestor->setAccessRightsFilter(
          Akonadi::Collection::CanCreateCollection |
          Akonadi::Collection::CanCreateItem );
}

KSelFilterPage::~KSelFilterPage() {
	qDeleteAll(mFilterList);
	mFilterList.clear();
}

void KSelFilterPage::filterSelected(int i)
{
	QString info = mFilterList.at(i)->info();
	const QString author = mFilterList.at(i)->author();
	if(!author.isEmpty())
		info += i18n("<p><i>Written by %1.</i></p>", author);
	mDesc->setText(info);
}

void KSelFilterPage::addFilter(Filter *f)
{
	mFilterList.append(f);
	mFilterCombo->addItem(f->name());
	if (mFilterCombo->count() == 1) filterSelected(0); // Setup description box with fist filter selected
}

bool KSelFilterPage::removeDupMsg_checked() const
{
        return remDupMsg->isChecked();
}

Filter * KSelFilterPage::getSelectedFilter(void)
{
	return mFilterList.at(mFilterCombo->currentIndex());
}

#include "kselfilterpage.moc"


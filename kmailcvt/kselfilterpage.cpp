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
#include <filter_mbox.h>
#include <filter_oe.h>
#include <filter_outlook.h>
#include <filter_pmail.h>
#include <filter_plain.h>
#include <filter_evolution.h>
#include <filter_mailapp.h>
#include <filter_evolution_v2.h>
#include <filter_opera.h>
#include <filter_thunderbird.h>
#include <filter_kmail_maildir.h>
#include <filter_kmail_archive.h>
#include <filter_sylpheed.h>
#include <filter_thebat.h>
#include <filter_lnotes.h>

#include <filters.h>

#include <filter_evolution_v3.h>

// KDE includes
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>

// Qt includes
#include <QCheckBox>

// Akonadi includes
#include <akonadi/collectionrequester.h>
#include <akonadi/control.h>

using namespace MailImporter;

KSelFilterPage::KSelFilterPage(QWidget *parent )
  : KSelFilterPageDlg(parent)
{
  mIntroSidebar->setPixmap(KStandardDirs::locate("data", "kmailcvt/pics/step1.png"));
  connect(mFilterCombo, SIGNAL(activated(int)), SLOT(filterSelected(int)));

  // Add new filters below. If this annoys you, please rewrite the stuff to use a factory.
  // The former approach was overengineered and only worked around problems in the design
  // For now, we have to live without the warm and fuzzy feeling a refactoring might give.
  // Patches appreciated. (danimo)

  addFilter(new MailImporter::FilterKMailArchive);
  addFilter(new MailImporter::FilterMBox);
  addFilter(new MailImporter::FilterEvolution);
  addFilter(new MailImporter::FilterEvolution_v2);
  addFilter(new MailImporter::FilterEvolution_v3);
  addFilter(new MailImporter::FilterKMail_maildir);
  addFilter(new MailImporter::FilterMailApp);
  addFilter(new MailImporter::FilterOpera);
  addFilter(new MailImporter::FilterSylpheed);
  addFilter(new MailImporter::FilterThunderbird);
  addFilter(new MailImporter::FilterTheBat);
  addFilter(new MailImporter::FilterOE);
  //        addFilter(new FilterOutlook);
  addFilter(new MailImporter::FilterPMail);
  addFilter(new MailImporter::FilterLNotes);
  addFilter(new MailImporter::FilterPlain);

  // Ensure we return the correct type of Akonadi collection.
  mCollectionRequestor->setMimeTypeFilter( QStringList() << QString( "message/rfc822" ) );
  mCollectionRequestor->setCollection(Akonadi::Collection());
  mCollectionRequestor->setAccessRightsFilter(
    Akonadi::Collection::CanCreateCollection |
    Akonadi::Collection::CanCreateItem );
  mCollectionRequestor->changeCollectionDialogOptions( Akonadi::CollectionDialog::AllowToCreateNewChildCollection );
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
  if (mFilterCombo->count() == 1)
    filterSelected(0); // Setup description box with fist filter selected
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


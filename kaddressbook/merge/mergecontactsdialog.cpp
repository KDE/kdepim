/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mergecontactsdialog.h"
#include "utils.h"
#include "util/mergecontactutil.h"
#include "mergecontactwidget.h"
#include "mergecontactinfowidget.h"
#include "merge/job/mergecontactsjob.h"
#include "mergecontactshowresultdialog.h"

#include <Akonadi/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KMessageBox>

#include <QItemSelectionModel>
#include <QLabel>
#include <QSplitter>
#include <QHBoxLayout>
#include <QPointer>

using namespace KABMergeContacts;
MergeContactsDialog::MergeContactsDialog(const Akonadi::Item::List &lst, QWidget *parent)
    : KDialog(parent),
      mContactWidget(0)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    readConfig();

    if (lst.count() < 2) {
        setMainWidget(new QLabel(i18n("You must select at least two elements.")));
    } else {
        if (!MergeContactUtil::hasSameNames(lst)) {
            setMainWidget(new QLabel(i18n("You selected %1 and some item has not the same name", lst.count())));
        } else {
            QSplitter *mainWidget = new QSplitter;
            mainWidget->setChildrenCollapsible(false);
            mContactWidget = new MergeContactWidget(lst);
            mainWidget->addWidget(mContactWidget);
            MergeContactInfoWidget *contactInfo = new MergeContactInfoWidget;
            mainWidget->addWidget(contactInfo);
            connect(mContactWidget, SIGNAL(contactSelected(Akonadi::Item)), contactInfo, SLOT(setContact(Akonadi::Item)));
            connect(mContactWidget, SIGNAL(mergeContact(Akonadi::Item::List,Akonadi::Collection)), this, SLOT(slotMergeContact(Akonadi::Item::List,Akonadi::Collection)));
            setMainWidget(mainWidget);
        }
    }
}

MergeContactsDialog::~MergeContactsDialog()
{
    writeConfig();
}

void MergeContactsDialog::slotMergeContact(const Akonadi::Item::List &lst, const Akonadi::Collection &col)
{
    if (lst.isEmpty()) {
        return;
    }
    enableButton(Close, false);
    MergeContactsJob *job = new MergeContactsJob(this);
    connect(job,SIGNAL(finished(Akonadi::Item)), this, SLOT(slotMergeContactFinished(Akonadi::Item)));
    job->setDestination(col);
    job->setListItem(lst);
    job->start();
}

void MergeContactsDialog::slotMergeContactFinished(const Akonadi::Item &item)
{
    if (!item.isValid()) {
        KMessageBox::error(this, i18n("Error during merge contacts."), i18n("Merge contact"));
    } else {
        mContactWidget->clear();
        QPointer<MergeContactShowResultDialog> dlg = new MergeContactShowResultDialog(this);
        Akonadi::Item::List lst;
        lst << item;
        dlg->setContacts(lst);
        dlg->exec();
        delete dlg;
    }
    enableButton(Close, true);
}

void MergeContactsDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactsDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

#include "moc_mergecontactsdialog.cpp"

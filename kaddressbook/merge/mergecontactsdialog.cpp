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
#include "mergecontactutil.h"
#include "mergecontactwidget.h"
#include "mergecontactsjob.h"

#include <Akonadi/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

#include <QItemSelectionModel>
#include <QLabel>
using namespace KABMergeContacts;
MergeContactsDialog::MergeContactsDialog(QItemSelectionModel *selectionModel, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    readConfig();

    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(selectionModel);
    if (lst.count() < 2) {
        setMainWidget(new QLabel(i18n("You must select at least two elements.")));
    } else {
        if (!MergeContactUtil::hasSameNames(lst)) {
            setMainWidget(new QLabel(i18n("You selected %1 and some item has not the same name", lst.count())));
        } else {
            MergeContactWidget *contactWidget = new MergeContactWidget(lst);
            connect(contactWidget, SIGNAL(mergeContact(Akonadi::Item::List,Akonadi::Collection)), this, SLOT(slotMergeContact(Akonadi::Item::List,Akonadi::Collection)));
            setMainWidget(contactWidget);
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
    MergeContactsJob *job = new MergeContactsJob(this);
    connect(job,SIGNAL(finished()), this, SLOT(slotMergeContactFinished()));
    job->setDestination(col);
    job->setListItem(lst);
    job->start();
}

void MergeContactsDialog::slotMergeContactFinished()
{
    //TODO
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

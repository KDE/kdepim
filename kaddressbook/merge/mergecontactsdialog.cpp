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

#include <AkonadiCore/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KMessageBox>

#include <QLabel>
#include <QSplitter>
#include <QPointer>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KABMergeContacts;
MergeContactsDialog::MergeContactsDialog(const Akonadi::Item::List &lst, QWidget *parent)
    : QDialog(parent),
      mContactWidget(0)
{
    setWindowTitle(i18n("Select Contacts to merge"));
    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &MergeContactsDialog::accept);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &MergeContactsDialog::reject);
    readConfig();

    if (lst.count() < 2) {
        mainLayout->addWidget(new QLabel(i18n("You must select at least two elements.")));
    } else {
        if (!MergeContactUtil::hasSameNames(lst)) {
            mainLayout->addWidget(new QLabel(i18n("You selected %1 and some item has not the same name", lst.count())));
        } else {
            QSplitter *mainWidget = new QSplitter;
            mainWidget->setChildrenCollapsible(false);
            mContactWidget = new MergeContactWidget(lst);
            mainWidget->addWidget(mContactWidget);
            MergeContactInfoWidget *contactInfo = new MergeContactInfoWidget;
            mainWidget->addWidget(contactInfo);
            connect(mContactWidget, &MergeContactWidget::contactSelected, contactInfo, &MergeContactInfoWidget::setContact);
            connect(mContactWidget, &MergeContactWidget::mergeContact, this, &MergeContactsDialog::slotMergeContact);
            mainLayout->addWidget(mainWidget);
        }
    }
    mainLayout->addWidget(mButtonBox);

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
    mButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    MergeContactsJob *job = new MergeContactsJob(this);
    connect(job, &MergeContactsJob::finished, this, &MergeContactsDialog::slotMergeContactFinished);
    job->setDestination(col);
    job->setListItem(lst);
    job->start();
}

void MergeContactsDialog::slotMergeContactFinished(const Akonadi::Item &item)
{
    if (!item.isValid()) {
        KMessageBox::error(this, i18n("Error while merging contacts."), i18n("Merge contact"));
    } else {
        mContactWidget->clear();
        QPointer<MergeContactShowResultDialog> dlg = new MergeContactShowResultDialog(this);
        Akonadi::Item::List lst;
        lst << item;
        dlg->setContacts(lst);
        dlg->exec();
        delete dlg;
    }
    mButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
}

void MergeContactsDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactsDialog");
    const QSize size = grp.readEntry("Size", QSize(300, 200));
    if (size.isValid()) {
        resize(size);
    }
}

void MergeContactsDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactsDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}


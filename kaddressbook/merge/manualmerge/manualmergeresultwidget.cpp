/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "manualmergeresultwidget.h"
#include "merge/widgets/mergecontactinfowidget.h"
#include "merge/job/mergecontactsjob.h"
#include "mergecontactwidget.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <KMessageBox>
#include <KLocalizedString>

using namespace KABMergeContacts;
ManualMergeResultWidget::ManualMergeResultWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    mSplitter = new QSplitter;
    mSplitter->setObjectName(QLatin1String("splitter"));
    mSplitter->setChildrenCollapsible(false);
    mainLayout->addWidget(mSplitter);

    mContactWidget = new MergeContactWidget;
    mContactWidget->setObjectName(QLatin1String("mergecontactwidget"));
    mSplitter->addWidget(mContactWidget);

    mMergeContactInfoWidget = new MergeContactInfoWidget;
    mMergeContactInfoWidget->setObjectName(QLatin1String("mergecontactinfowidget"));
    mSplitter->addWidget(mMergeContactInfoWidget);
    connect(mContactWidget, SIGNAL(contactSelected(Akonadi::Item)), mMergeContactInfoWidget, SLOT(setContact(Akonadi::Item)));
    connect(mContactWidget, SIGNAL(mergeContact(Akonadi::Item::List,Akonadi::Collection)), this, SLOT(slotMergeContact(Akonadi::Item::List,Akonadi::Collection)));
}

ManualMergeResultWidget::~ManualMergeResultWidget()
{

}

void ManualMergeResultWidget::setContacts(const Akonadi::Item::List &list)
{
    mContactWidget->setContacts(list);
}

void ManualMergeResultWidget::slotMergeContact(const Akonadi::Item::List &lst, const Akonadi::Collection &col)
{
    if (lst.isEmpty()) {
        return;
    }
    //enableButton(Close, false);
    MergeContactsJob *job = new MergeContactsJob(this);
    connect(job,SIGNAL(finished(Akonadi::Item)), this, SLOT(slotMergeContactFinished(Akonadi::Item)));
    job->setDestination(col);
    job->setListItem(lst);
    job->start();
}

void ManualMergeResultWidget::slotMergeContactFinished(const Akonadi::Item &item)
{
    if (!item.isValid()) {
        KMessageBox::error(this, i18n("Error during merge contacts."), i18n("Merge contact"));
    } else {
#if 0
        mContactWidget->clear();
        QPointer<MergeContactShowResultDialog> dlg = new MergeContactShowResultDialog(this);
        Akonadi::Item::List lst;
        lst << item;
        dlg->setContacts(lst);
        dlg->exec();
        delete dlg;
#endif
    }
    //enableButton(Close, true);
}



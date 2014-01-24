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

#include "storageservicedownloaddialog.h"
#include "storageservice/widgets/storageservicetreewidget.h"

#include "storageservice/storageserviceabstract.h"

#include <KLocalizedString>

#include <QGridLayout>
#include <QLabel>
#include <QTreeWidget>

using namespace PimCommon;

StorageServiceDownloadDialog::StorageServiceDownloadDialog(PimCommon::StorageServiceAbstract *storage, const QString &destination, QWidget *parent)
    : KDialog(parent),
      mDestination(destination),
      mStorage(storage)
{
    setCaption( i18n( "Download File" ) );
    setButtons( User1 | Cancel );

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Select file to download:"));
    vbox->addWidget(lab);

    mTreeWidget = new StorageServiceTreeWidget(storage);
    mTreeWidget->refreshList();
    connect(mTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));

    vbox->addWidget(mTreeWidget);

    w->setLayout(vbox);
    setMainWidget(w);
    enableButton(User1, false);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotDownloadFile()));
}

StorageServiceDownloadDialog::~StorageServiceDownloadDialog()
{

}

void StorageServiceDownloadDialog::slotItemActivated(QTreeWidgetItem *item, int)
{
    enableButton(User1, (item && (mTreeWidget->type(item) == StorageServiceTreeWidget::File)));
}

void StorageServiceDownloadDialog::slotDownloadFile()
{
    //TODO mStorage->downloadFile();
}

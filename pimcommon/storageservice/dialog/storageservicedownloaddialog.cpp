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
#include "storageservice/widgets/storageserviceprogresswidget.h"
#include "storageservice/storageserviceabstract.h"

#include <KLocalizedString>
#include <KGlobal>
#include <KSharedConfig>
#include <KMessageBox>
#include <KFileDialog>

#include <QGridLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QFileInfo>

using namespace PimCommon;

StorageServiceDownloadDialog::StorageServiceDownloadDialog(PimCommon::StorageServiceAbstract *storage, QWidget *parent)
    : KDialog(parent),
      mStorage(storage)
{
    setCaption( i18n( "Download File" ) );

    setButtons( User1 | Close );
    setButtonText(User1, i18n("Download"));

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Select file to download:"));
    vbox->addWidget(lab);

    mTreeWidget = new StorageServiceTreeWidget(storage);


    vbox->addWidget(mTreeWidget);
    mProgressWidget = new StorageServiceProgressWidget(storage);
    mProgressWidget->setProgressBarType(StorageServiceProgressWidget::DownloadBar);
    vbox->addWidget(mProgressWidget);
    mProgressWidget->hide();
    w->setLayout(vbox);
    setMainWidget(w);
    enableButton(User1, false);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotDownloadFile()));
    connect(mStorage, SIGNAL(listFolderDone(QString,QString)), mTreeWidget, SLOT(slotListFolderDone(QString,QString)));
    connect(mStorage, SIGNAL(downLoadFileDone(QString,QString)), this, SLOT(slotDownfileDone(QString,QString)));
    connect(mStorage, SIGNAL(downLoadFileFailed(QString,QString)), this, SLOT(slotDownfileFailed(QString,QString)));
    connect(mTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
    mTreeWidget->refreshList();
    readConfig();
}

StorageServiceDownloadDialog::~StorageServiceDownloadDialog()
{
    writeConfig();
}

void StorageServiceDownloadDialog::setDefaultDownloadPath(const QString &path)
{
    mDefaultDownloadPath = path;
}

void StorageServiceDownloadDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "StorageServiceDownloadDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServiceDownloadDialog::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServiceDownloadDialog") );
    group.writeEntry( "Size", size() );
}

void StorageServiceDownloadDialog::slotItemActivated(QTreeWidgetItem *item, int)
{
    enableButton(User1, (item && (mTreeWidget->type(item) == StorageServiceTreeWidget::File)));
}

void StorageServiceDownloadDialog::slotDownloadFile()
{
    const QString filename = mTreeWidget->currentItem()->text(0);
    if (!filename.isEmpty()) {
        QString destination = mDefaultDownloadPath;
        if (destination.isEmpty())
            destination = KFileDialog::getExistingDirectory(KUrl(), this);
        if (destination.isEmpty())
            return;
        QFileInfo fileInfo(destination + QLatin1Char('/') + filename);
        if (fileInfo.exists()) {
            if (KMessageBox::No == KMessageBox::questionYesNo(this, i18n("Filename already exists. Do you want to overwrite it?"), i18n("Overwrite file"))) {
                return;
            }
        }
        const QString fileId = mStorage->fileIdentifier(mTreeWidget->itemInformationSelected());
        mTreeWidget->setEnabled(false);
        mStorage->downloadFile(filename, fileId, destination);
    }
}

void StorageServiceDownloadDialog::slotDownfileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(filename);
    KMessageBox::information(this, i18n("File was correctly downloaded."), i18n("Download file"));
    mTreeWidget->setEnabled(true);
}

void StorageServiceDownloadDialog::slotDownfileFailed(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(filename);
    KMessageBox::information(this, i18n("Error during download file."), i18n("Download file"));
    mTreeWidget->setEnabled(true);
}

#include "moc_storageservicedownloaddialog.cpp"

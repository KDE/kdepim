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
#include "storageservice/widgets/storageservicetreewidgetitem.h"

#include "storageservice/widgets/storageserviceprogresswidget.h"
#include "storageservice/widgets/storageserviceprogressindicator.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/storageserviceprogressmanager.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KMessageBox>
#include <KFileDialog>
#include <QMenu>
#include <KUrl>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QFileInfo>
#include <QHeaderView>
#include <QCloseEvent>
#include <QFileDialog>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace PimCommon;


StorageServiceDownloadTreeWidget::StorageServiceDownloadTreeWidget(PimCommon::StorageServiceAbstract *storageService, QWidget *parent)
    : PimCommon::StorageServiceTreeWidget(storageService, parent)
{
}

void StorageServiceDownloadTreeWidget::createMenuActions(QMenu *menu)
{
    createUpAction(menu);
    menu->addSeparator();
    const PimCommon::StorageServiceTreeWidget::ItemType type = StorageServiceTreeWidget::itemTypeSelected();
    if (type == StorageServiceTreeWidget::File)
        menu->addAction(QIcon::fromTheme(QLatin1String("download")), i18n("Download File"), this, SIGNAL(downloadFile()));
    if ((type == StorageServiceTreeWidget::File) || (type == StorageServiceTreeWidget::Folder)) {
        menu->addSeparator();
        createPropertiesAction(menu);
    }
}


StorageServiceDownloadDialog::StorageServiceDownloadDialog(PimCommon::StorageServiceAbstract *storage, QWidget *parent)
    : QDialog(parent),
      mStorage(storage)
{
    setWindowTitle( i18n( "Download File" ) );

    mStorageServiceProgressIndicator = new PimCommon::StorageServiceProgressIndicator(this);
    connect(mStorageServiceProgressIndicator, SIGNAL(updatePixmap(QPixmap)), this, SLOT(slotUpdatePixmap(QPixmap)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mUser1Button = new QPushButton;
    mCloseButton = buttonBox->button(QDialogButtonBox::Close);
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mUser1Button->setText(i18n("Download"));

    QWidget *w = new QWidget;
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Select file to download:"));
    hbox->addWidget(lab);
    mLabelProgressIncator = new QLabel;
    hbox->addWidget(mLabelProgressIncator);
    hbox->setAlignment(mLabelProgressIncator, Qt::AlignLeft);
    mTreeWidget = new StorageServiceDownloadTreeWidget(storage);
    connect(mTreeWidget, SIGNAL(downloadFile()), this, SLOT(slotDownloadFile()));


    vbox->addWidget(mTreeWidget);
    connect(mTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
    mProgressWidget = new StorageServiceProgressWidget(storage);
    mProgressWidget->setProgressBarType(StorageServiceProgressWidget::DownloadBar);
    vbox->addWidget(mProgressWidget);
    mProgressWidget->hide();
    w->setLayout(vbox);
    mainLayout->addWidget(w);
    mUser1Button->setEnabled(false);
    connect(mUser1Button, SIGNAL(clicked()), this, SLOT(slotDownloadFile()));
    connect(mStorage, SIGNAL(listFolderDone(QString,QVariant)), this, SLOT(slotListFolderDone(QString,QVariant)));
    connect(mStorage, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString,QString)));
    connect(mStorage, SIGNAL(downLoadFileDone(QString,QString)), this, SLOT(slotDownfileDone(QString,QString)));
    connect(mStorage, SIGNAL(downLoadFileFailed(QString,QString)), this, SLOT(slotDownfileFailed(QString,QString)));
    connect(mStorage, SIGNAL(uploadDownloadFileProgress(QString,qint64,qint64)), this, SLOT(slotUploadDownloadFileProgress(QString,qint64,qint64)));
    connect(mTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
    mStorageServiceProgressIndicator->startAnimation();
    mTreeWidget->setEnabled(false);
    mTreeWidget->refreshList();
    readConfig();
}

StorageServiceDownloadDialog::~StorageServiceDownloadDialog()
{
    writeConfig();
}

void StorageServiceDownloadDialog::closeEvent(QCloseEvent *e)
{
    if (mStorage->hasUploadOrDownloadInProgress()) {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("A download is still in progress. Do you want to cancel it and close dialog?"), i18n("Download in progress"))) {
            mStorage->cancelDownloadFile();
            e->accept();
        } else {
            e->ignore();
            return;
        }
    } else {
        e->accept();
    }
}

void StorageServiceDownloadDialog::slotUploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    Q_UNUSED(serviceName);
    mProgressWidget->setProgressValue(done, total);
}

void StorageServiceDownloadDialog::reenableDialog()
{
    mStorageServiceProgressIndicator->stopAnimation();
    mTreeWidget->setEnabled(true);
    mProgressWidget->hide();
    mProgressWidget->reset();
    mUser1Button->setEnabled(true);
    mCloseButton->setEnabled(true);
}

void StorageServiceDownloadDialog::slotActionFailed(const QString &serviceName, const QString &data)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(data);
    reenableDialog();
}

void StorageServiceDownloadDialog::slotListFolderDone(const QString &serviceName, const QVariant &data)
{
    mTreeWidget->setEnabled(true);
    mStorageServiceProgressIndicator->stopAnimation();
    mTreeWidget->slotListFolderDone(serviceName, data);
}

void StorageServiceDownloadDialog::slotUpdatePixmap(const QPixmap &pix)
{
    mLabelProgressIncator->setPixmap(pix);
}

void StorageServiceDownloadDialog::setDefaultDownloadPath(const QString &path)
{
    mDefaultDownloadPath = path;
}

void StorageServiceDownloadDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageServiceDownloadDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    mTreeWidget->header()->restoreState( group.readEntry( mStorage->storageServiceName(), QByteArray() ) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServiceDownloadDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("StorageServiceDownloadDialog") );
    group.writeEntry( "Size", size() );
    group.writeEntry(mStorage->storageServiceName(), mTreeWidget->header()->saveState());
}

void StorageServiceDownloadDialog::slotItemActivated(QTreeWidgetItem *item, int)
{
    mUser1Button->setEnabled((item&&(mTreeWidget->type(item)==StorageServiceTreeWidget::File)));
}

void StorageServiceDownloadDialog::slotDownloadFile()
{
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem*>(mTreeWidget->currentItem());
    if (storageServiceItem) {
        downloadItem(storageServiceItem);
    }
}

void StorageServiceDownloadDialog::slotDownfileDone(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(filename);
    KMessageBox::information(this, i18n("File was correctly downloaded."), i18n("Download file"));
    reenableDialog();
}

void StorageServiceDownloadDialog::slotDownfileFailed(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(filename);
    KMessageBox::information(this, i18n("Error during download file."), i18n("Download file"));
    reenableDialog();
}

void StorageServiceDownloadDialog::slotItemDoubleClicked(QTreeWidgetItem *item, int)
{
    StorageServiceTreeWidgetItem *storageServiceItem = dynamic_cast<StorageServiceTreeWidgetItem*>(item);
    if (storageServiceItem) {
        if (mTreeWidget->type(storageServiceItem) == StorageServiceTreeWidget::File) {
            downloadItem(storageServiceItem);
        }
    }
}

void StorageServiceDownloadDialog::downloadItem(StorageServiceTreeWidgetItem *item)
{
    const QString filename = item->text(0);
    if (!filename.isEmpty()) {
        QString destination = mDefaultDownloadPath;
        if (destination.isEmpty() || !QFileInfo(mDefaultDownloadPath).isDir())
            destination = QFileDialog::getExistingDirectory(this, QString());
        if (destination.isEmpty())
            return;
        QFileInfo fileInfo(destination + QLatin1Char('/') + filename);
        if (fileInfo.exists()) {
            if (KMessageBox::No == KMessageBox::questionYesNo(this, i18n("Filename already exists. Do you want to overwrite it?"), i18n("Overwrite file"))) {
                return;
            }
        }
        PimCommon::StorageServiceProgressManager::self()->addProgress(mStorage, StorageServiceProgressManager::DownLoad);

        const QString fileId = mStorage->fileIdentifier(item->storeInfo());
        mTreeWidget->setEnabled(false);
        mUser1Button->setEnabled(false);
        mCloseButton->setEnabled(false);
        mProgressWidget->show();
        mStorage->downloadFile(filename, fileId, destination);
    }
}

#include "moc_storageservicedownloaddialog.cpp"

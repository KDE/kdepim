/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kmessagebox.h>

#include <ksyncer.h>
#include <calendarsyncee.h>
#include <bookmarksyncee.h>
#include <addressbooksyncee.h>
#include <ksyncuikde.h>

#include "ksync.h"

#include "ksyncview.h"
#include "ksyncview.moc"

class SynceeListItem : public QListViewItem {
  public:
    SynceeListItem(QListView *lv,KURL url) : QListViewItem(lv,url.url()),
      mUrl(url) {}

    void setSyncee(KSyncee *syncee) { mSyncee = syncee; }
    KSyncee *syncee() { return mSyncee; }

    KURL url() { return mUrl; }

  private:
    KSyncee *mSyncee;
    KURL mUrl;
};

KSyncView::KSyncView(QWidget *parent, const char *name) :
  QWidget(parent, name),
  mSyncer(0), mTarget(0), mLoadError(false)
{
  mTmpFiles.setAutoDelete(true);

  QLabel *typeLabel = new QLabel(i18n("Data type to be synced:"),this);

  mTypeCombo = new QComboBox(this);
  mTypeCombo->insertItem(i18n("Calendar"),TypeCalendar);
  mTypeCombo->insertItem(i18n("Bookmarks"),TypeBookmarks);
  mTypeCombo->insertItem(i18n("Addressbook"),TypeAddressBook);
  mCurrentType = mTypeCombo->currentItem();
  connect(mTypeCombo,SIGNAL(activated(int)),SLOT(reloadSyncees()));

  QPushButton *addButton = new QPushButton(i18n("Add Source..."),this);
  connect(addButton,SIGNAL(clicked()),SLOT(addSource()));

  removeButton = new QPushButton(i18n("Remove Source"),this);
  connect(removeButton,SIGNAL(clicked()),SLOT(removeSource()));

  showButton = new QPushButton(i18n("Show Source"),this);
  connect(showButton,SIGNAL(clicked()),SLOT(showSource()));

  mSourceListView = new QListView(this);
  mSourceListView->addColumn(i18n("URL"));

  connect(mSourceListView,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  mSyncBackCheck = new QCheckBox(i18n("Write synced data back to sources."),
                                 this);
  connect(mSyncBackCheck,SIGNAL(clicked()),SLOT(checkSyncBack()));

  QLabel *targetLabel = new QLabel(i18n("Target: "),this);
  mTargetReq = new KURLRequester(this);

  QPushButton *syncButton = new QPushButton(i18n("Sync"),this);
  connect(syncButton,SIGNAL(clicked()),SLOT(doSync()));

  checkSyncBack();

  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setMargin(8);
  topLayout->setSpacing(8);
  topLayout->addWidget(typeLabel,0,0);
  topLayout->addMultiCellWidget(mTypeCombo,0,0,1,2);
  topLayout->addMultiCellWidget(addButton,1,1,0,0);
  topLayout->addMultiCellWidget(removeButton,1,1,1,1);
  topLayout->addMultiCellWidget(showButton,1,1,2,2);
  topLayout->addMultiCellWidget(mSourceListView,2,2,0,2);
  topLayout->addMultiCellWidget(mSyncBackCheck,3,3,0,2);
  topLayout->addMultiCellWidget(targetLabel,4,4,0,0);
  topLayout->addMultiCellWidget(mTargetReq,4,4,1,2);
  topLayout->addMultiCellWidget(syncButton,5,5,0,2);
  slotSelectionChanged();
}

KSyncView::~KSyncView()
{
}

void KSyncView::print(QPrinter *pPrinter)
{
  QPainter printpainter;
  printpainter.begin(pPrinter);

  // TODO: add your printing code here

  printpainter.end();
}

void KSyncView::addSource()
{
  KURL url = KFileDialog::getOpenURL();
  if(!url.path().isEmpty())
    new SynceeListItem(mSourceListView,url);
}

void KSyncView::removeSource()
{
  QListViewItem *item = mSourceListView->selectedItem();
  if (item) delete item;
}

void KSyncView::showSource()
{
  QListViewItem *item = mSourceListView->selectedItem();
  if (!item) {
    kdDebug() << "No item selected" << endl;
    return;
  } else {
    kdDebug() << "** Source '" << item->text(0) << endl;
    KSyncee *syncee = createSyncee(KURL( item->text(0) ));
    KSyncEntry *entry = syncee->firstEntry();
    while(entry) {
      kdDebug() << "**  '" << entry->name() << "'" << endl;

      entry = syncee->nextEntry();
    }
    delete syncee;
  }
}

void KSyncView::slotSelectionChanged()
{
    bool state=(mSourceListView->currentItem()!=0);
    showButton->setEnabled(state);
    removeButton->setEnabled(state);
}

void KSyncView::doSync()
{
  delete mSyncer;
  mSyncer = new KSyncer(new KSyncUiKde(this));

  mLoadCount = 0;
  mLoadError = false;

  SynceeListItem *item = dynamic_cast<SynceeListItem *>(mSourceListView->firstChild());
  while(item) {
    KSyncee *syncee = createSyncee(item->url());
    item->setSyncee(syncee);
    mSyncer->addSyncee(syncee);

    item = (SynceeListItem *)item->nextSibling();
  }

  QString url = mTargetReq->url();

  kdDebug() << "target url: " << url << endl;
  mTarget = createSyncee(KURL(url));

  finishSync();
}

KSyncee *KSyncView::createSyncee(const KURL &url)
{
  kdDebug() << "KSyncView::createSyncee(): " << url.url() << endl;

  KSyncee *syncee;
  switch (mTypeCombo->currentItem()) {
    case TypeBookmarks:
      syncee = new BookmarkSyncee();
      break;
    case TypeAddressBook:
      syncee = new AddressBookSyncee();
      break;
    case TypeCalendar:
    default:
      syncee = new CalendarSyncee();
      break;
  }

  SynceeLoader *loader;
  if (url.isLocalFile()) {
    loader = new SynceeLoader(this,syncee,url.path());
    loader->loadSyncee();
    ++mLoadCount;
    delete loader;
    return syncee;
  } else {
    QString tmpFile = createTempFile();

    loader = new SynceeLoader(this,syncee,tmpFile);
    KIO::FileCopyJob *j = KIO::file_copy(url,KURL::fromPathOrURL( tmpFile ),-1,true);
    connect(j,SIGNAL(result(KIO::Job *)),
            loader,SLOT(loadSyncee(KIO::Job *)));
    return syncee;
  }
}

void KSyncView::synceeLoaded()
{
  ++mLoadCount;
  finishSync();
}

void KSyncView::finishSync()
{
  kdDebug() << "KSyncView::finishSync()" << endl;

  if (mLoadError) {
    kdDebug() << "KSyncView::finishSync(): load error" << endl;
    return;
  }

  if (mLoadCount == mSourceListView->childCount() + 1) {
    mLoadCount = 0;
    if (mSyncBackCheck->isChecked()) {
      mSyncer->sync();
      SynceeListItem *item = dynamic_cast<SynceeListItem *>(mSourceListView->firstChild());
      KIO::FileCopyJob *j;
      while(item) {
        KURL from(item->syncee()->filename());
        KURL to(item->url());
        if (from != to) {
          kdDebug() << "Copy " << from.url() << " to " << to.url() << endl;
          j = KIO::file_copy(from,to,-1,true);
          connect(j,SIGNAL(result(KIO::Job *)),SLOT(jobFinished(KIO::Job *)));
        } else {
          checkFinish();
        }

        item = (SynceeListItem *)item->nextSibling();
      }
    } else {
      mSyncer->syncAllToTarget(mTarget);
      mTarget->save();
    }
  } else {
    kdDebug() << "KSyncView::finishSync(): loadCount: " << mLoadCount << " childCount: "
              << mSourceListView->childCount() + 1 << endl;
  }
}

void KSyncView::jobFinished(KIO::Job *job)
{
  if (job->error()) {
    job->showErrorDialog(this);
  } else {
    checkFinish();
  }
}

void KSyncView::checkFinish()
{
  ++mLoadCount;
  if (mLoadCount == mSourceListView->childCount()) {
    KMessageBox::information(this,i18n("Sync finished"));
    mLoadCount = 0;
  }
}

void KSyncView::synceeLoadError()
{
  kdDebug() << "KSyncView::synceeLoadError()" << endl;

  mLoadError = true;

  KMessageBox::error(this,i18n("Can't load syncee."),i18n("Load Error"));
}

void KSyncView::readConfig(KConfig *config)
{
  int typeIndex = config->readNumEntry("SyncType",TypeCalendar);
  mTypeCombo->setCurrentItem(typeIndex);
  mCurrentType = typeIndex;

  readTypeConfig(config);
}

void KSyncView::readTypeConfig(KConfig *config)
{
  QString typeString = mTypeCombo->text(mCurrentType);

  QStringList sources = config->readPathListEntry("Sources_" + typeString);

  mSourceListView->clear();
  QStringList::ConstIterator it = sources.begin();
  while(it != sources.end()) {
    new SynceeListItem (mSourceListView,KURL(*it));
    ++it;
  }

  mTargetReq->setURL(config->readPathEntry("Target_" + typeString));
}

void KSyncView::writeConfig(KConfig *config)
{
  config->writeEntry("SyncType",mTypeCombo->currentItem());

  writeTypeConfig(config);
}

void KSyncView::writeTypeConfig(KConfig *config)
{
  QStringList sources;
  QListViewItem *item = mSourceListView->firstChild();
  while(item) {
    sources.append(item->text(0));
    item = item->nextSibling();
  }

  QString typeString = mTypeCombo->text(mCurrentType);

  config->writePathEntry("Sources_" + typeString,sources);
  config->writePathEntry("Target_" + typeString,mTargetReq->url());

  config->sync();
}

void KSyncView::checkSyncBack()
{
  if (mSyncBackCheck->isChecked()) {
    mTargetReq->setEnabled(false);
  } else {
    mTargetReq->setEnabled(true);
  }
}

void KSyncView::reloadSyncees()
{
  KConfig *config = kapp->config();

  writeTypeConfig(config);
  mCurrentType = mTypeCombo->currentItem();
  readTypeConfig(config);
}

QString KSyncView::createTempFile()
{
  KTempFile *tmpFile = new KTempFile;
  mTmpFiles.append(tmpFile);
  tmpFile->setAutoDelete(true);
  return tmpFile->name();
}

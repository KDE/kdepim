/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "archivemaildialog.h"
#include "addarchivemaildialog.h"

#include <QHBoxLayout>

ArchiveMailDialog::ArchiveMailDialog(QWidget *parent)
  :KDialog(parent)
{
  setCaption( i18n( "Configure Archive Mail Agent" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QWidget *mainWidget = new QWidget( this );
  QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( KDialog::marginHint() );
  mWidget = new ArchiveMailWidget(this);
  mainLayout->addWidget(mWidget);
  setMainWidget( mainWidget );
  connect(this,SIGNAL(okClicked()),SLOT(slotSave()));
}

ArchiveMailDialog::~ArchiveMailDialog()
{

}

void ArchiveMailDialog::slotSave()
{
  mWidget->save();
}


ArchiveMailItem::ArchiveMailItem( const QString &text, QListWidget *parent )
  : QListWidgetItem(text,parent),mInfo(0)
{
}

ArchiveMailItem::~ArchiveMailItem()
{
  delete mInfo;
}

void ArchiveMailItem::setInfo(ArchiveMailInfo* info)
{
  mInfo = info;
}

ArchiveMailInfo* ArchiveMailItem::info() const
{
  return mInfo;
}


ArchiveMailWidget::ArchiveMailWidget( QWidget *parent )
  : QWidget( parent )
{
  mWidget = new Ui::ArchiveMailWidget;
  mWidget->setupUi( this );
  load();
  connect(mWidget->removeItem,SIGNAL(clicked(bool)),SLOT(slotRemoveItem()));
  connect(mWidget->modifyItem,SIGNAL(clicked(bool)),SLOT(slotModifyItem()));
  connect(mWidget->addItem,SIGNAL(clicked(bool)),SLOT(slotAddItem()));
  connect(mWidget->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),SLOT(updateButtons()));
  connect(mWidget->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),SLOT(slotModifyItem()));
  updateButtons();
}

ArchiveMailWidget::~ArchiveMailWidget()
{
  delete mWidget;
}

void ArchiveMailWidget::updateButtons()
{
  if(!mWidget->listWidget->currentItem()) {
    mWidget->removeItem->setEnabled(true);
    mWidget->modifyItem->setEnabled(true);
  } else {
    mWidget->removeItem->setEnabled(false);
    mWidget->modifyItem->setEnabled(false);
  }
}

void ArchiveMailWidget::load()
{
  KSharedConfig::Ptr config = KGlobal::config();
  const QStringList collectionList = config->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
  qDebug()<<"collectionList "<<collectionList;
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);
    ArchiveMailItem *item = new ArchiveMailItem(i18n("Folder: %1",QString::number(info->saveCollectionId())), mWidget->listWidget);
    item->setInfo(info);
  }
  updateButtons();
}

void ArchiveMailWidget::save()
{
  KSharedConfig::Ptr config = KGlobal::config();
  const int numberOfItem(mWidget->listWidget->count());
  for(int i = 0; i < numberOfItem; ++i) {
    ArchiveMailItem *mailItem = static_cast<ArchiveMailItem *>(mWidget->listWidget->item(i));
    if(mailItem->info()) {
      KConfigGroup group = config->group(QString::fromLatin1("ArchiveMailCollection %1").arg(mailItem->info()->saveCollectionId()));
      mailItem->info()->writeConfig(group);
    }
  }
  config->sync();
}

void ArchiveMailWidget::slotRemoveItem()
{
  if(!mWidget->listWidget->currentItem())
    return;
  delete mWidget->listWidget->takeItem(mWidget->listWidget->currentRow());
  updateButtons();
}

void ArchiveMailWidget::slotModifyItem()
{
  QListWidgetItem *item = mWidget->listWidget->currentItem();
  if(!item)
    return;
  ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
  AddArchiveMailDialog *dialog = new AddArchiveMailDialog(archiveItem->info(), this);
  if( dialog->exec() ) {
    //TODO fix item name
    archiveItem->setInfo(dialog->info());
  }
  delete dialog;
}

void ArchiveMailWidget::slotAddItem()
{
  AddArchiveMailDialog *dialog = new AddArchiveMailDialog(0,this);
  if( dialog->exec() ) {
    ArchiveMailInfo *info = dialog->info();
    //FIXME item name
    ArchiveMailItem *item = new ArchiveMailItem(i18n("Folder: %1",QString::number(info->saveCollectionId())), mWidget->listWidget);
    item->setInfo(info);
    updateButtons();
  }
  delete dialog;
}

#include "archivemaildialog.moc"

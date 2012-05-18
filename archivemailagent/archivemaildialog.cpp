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
#include <mailcommon/mailutil.h>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <QHBoxLayout>

static QString archiveMailCollectionPattern = QLatin1String( "ArchiveMailCollection \\d+" );

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


ArchiveMailItem::ArchiveMailItem(QTreeWidget *parent )
  : QTreeWidgetItem(parent),mInfo(0)
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
  QStringList headers;
  headers<<i18n("Name")<<i18n("Last archive")<<i18n("Next archive in");
  mWidget->treeWidget->setHeaderLabels(headers);
  load();
  connect(mWidget->removeItem,SIGNAL(clicked(bool)),SLOT(slotRemoveItem()));
  connect(mWidget->modifyItem,SIGNAL(clicked(bool)),SLOT(slotModifyItem()));
  connect(mWidget->addItem,SIGNAL(clicked(bool)),SLOT(slotAddItem()));
  connect(mWidget->treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),SLOT(updateButtons()));
  connect(mWidget->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),SLOT(slotModifyItem()));
  updateButtons();
}

ArchiveMailWidget::~ArchiveMailWidget()
{
  delete mWidget;
}

void ArchiveMailWidget::updateButtons()
{
  if(mWidget->treeWidget->currentItem()) {
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
  const QStringList collectionList = config->groupList().filter( QRegExp( archiveMailCollectionPattern ) );
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);
    createOrUpdateItem(info);
  }
  updateButtons();
}

void ArchiveMailWidget::createOrUpdateItem(ArchiveMailInfo *info, ArchiveMailItem* item)
{
  if(!item) {
    item = new ArchiveMailItem(mWidget->treeWidget);
  }
  item->setText(0,i18n("Folder: %1",MailCommon::Util::fullCollectionPath(Akonadi::Collection(info->saveCollectionId()))));
  item->setText(1,KGlobal::locale()->formatDate(info->lastDateSaved()));
  QDate diffDate(info->lastDateSaved());
  switch(info->archiveUnit()) {
    case ArchiveMailInfo::ArchiveDays:
      diffDate = diffDate.addDays(info->archiveAge());
      break;
    case ArchiveMailInfo::ArchiveWeeks:
      diffDate = diffDate.addDays(info->archiveAge()*7);
      break;
    case ArchiveMailInfo::ArchiveMonths:
      diffDate = diffDate.addMonths(info->archiveAge());
      break;
    default:
      qDebug()<<"archiveUnit not defined :"<<info->archiveUnit();
      break;
  }
  item->setText(2,i18np("1 day", "%1 days",QString::number(info->lastDateSaved().daysTo(diffDate))));
  item->setInfo(info);
}

void ArchiveMailWidget::save()
{
  KSharedConfig::Ptr config = KGlobal::config();

  // first, delete all filter groups:
  const QStringList filterGroups =config->groupList().filter( QRegExp( archiveMailCollectionPattern ) );

  foreach ( const QString &group, filterGroups ) {
    config->deleteGroup( group );
  }

  const int numberOfItem(mWidget->treeWidget->topLevelItemCount());
  for(int i = 0; i < numberOfItem; ++i) {
    ArchiveMailItem *mailItem = static_cast<ArchiveMailItem *>(mWidget->treeWidget->topLevelItem(i));
    if(mailItem->info()) {
      KConfigGroup group = config->group(QString::fromLatin1("ArchiveMailCollection %1").arg(mailItem->info()->saveCollectionId()));
      mailItem->info()->writeConfig(group);
    }
  }
  config->sync();
}

void ArchiveMailWidget::slotRemoveItem()
{
  QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
  if(KMessageBox::warningYesNo(this,i18n("Do you want to delete selected items? Do you want to continue?"),i18n("Remove items"))== KMessageBox::No)
    return;

  Q_FOREACH(QTreeWidgetItem *item,listItems) {
    delete item;
  }
  updateButtons();
}

void ArchiveMailWidget::slotModifyItem()
{
  QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
  if(listItems.count()==1) {
    QTreeWidgetItem *item = listItems.at(0);
    if(!item)
      return;
    ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
    AddArchiveMailDialog *dialog = new AddArchiveMailDialog(archiveItem->info(), this);
    if( dialog->exec() ) {
      ArchiveMailInfo *info = dialog->info();
      createOrUpdateItem(info,archiveItem);
    }
    delete dialog;
  }
}

void ArchiveMailWidget::slotAddItem()
{
  AddArchiveMailDialog *dialog = new AddArchiveMailDialog(0,this);
  if( dialog->exec() ) {
    ArchiveMailInfo *info = dialog->info();
    createOrUpdateItem(info);
    updateButtons();
  }
  delete dialog;
}

#include "archivemaildialog.moc"

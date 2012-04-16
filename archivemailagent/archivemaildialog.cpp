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

ArchiveMailDialog::ArchiveMailDialog(QWidget *parent)
  :KDialog(parent)
{
  setCaption( i18n( "Configure Archive Mail Agent" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QWidget *mainWidget = new QWidget( this );
  QGridLayout *mainLayout = new QGridLayout( mainWidget );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( KDialog::marginHint() );
  mWidget = new ArchiveMailWidget(this);
  mainLayout->addWidget(mWidget);
  setMainWidget( mainWidget );
}

ArchiveMailDialog::~ArchiveMailDialog()
{

}


ArchiveMailItem::ArchiveMailItem( const QString &text, QListWidget *parent )
  : QListWidgetItem(text,parent)
{
}

ArchiveMailItem::~ArchiveMailItem()
{
}

void ArchiveMailItem::setInfo(const ArchiveMailInfo& info)
{
  mInfo = info;
}

ArchiveMailInfo ArchiveMailItem::info() const
{
  return mInfo;
}


ArchiveMailWidget::ArchiveMailWidget( QWidget *parent )
  : QWidget( parent )
{
  mWidget = new Ui::ArchiveMailWidget;
  mWidget->setupUi( this );
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
  //TODO
}

void ArchiveMailWidget::save()
{
  //TODO
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
  //TODO MODIFY it
  if( dialog->exec() ) {
  }
  delete dialog;
}

void ArchiveMailWidget::slotAddItem()
{
  //FIXME
  ArchiveMailInfo archiveInfo;
  AddArchiveMailDialog *dialog = new AddArchiveMailDialog(archiveInfo,this);
  if( dialog->exec() ) {
    //FIXME
   ArchiveMailItem *item = new ArchiveMailItem(i18n("foo"), mWidget->listWidget);
   item->setInfo(archiveInfo);
   //TODO
   updateButtons();
  }
  delete dialog;
}

#include "archivemaildialog.moc"

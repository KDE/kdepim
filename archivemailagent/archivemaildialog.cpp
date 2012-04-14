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

ArchiveMailWidget::ArchiveMailWidget( QWidget *parent )
  : QWidget( parent )
{
  mWidget = new Ui::ArchiveMailWidget;
  mWidget->setupUi( this );
  connect(mWidget->removeItem,SIGNAL(clicked(bool)),SLOT(slotRemoveItem()));
  connect(mWidget->modifyItem,SIGNAL(clicked(bool)),SLOT(slotModifyItem()));
  connect(mWidget->addItem,SIGNAL(clicked(bool)),SLOT(slotAddItem()));
}

ArchiveMailWidget::~ArchiveMailWidget()
{
  delete mWidget;
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
}

void ArchiveMailWidget::slotModifyItem()
{
  if(!mWidget->listWidget->currentItem())
    return;
//TODO
}

void ArchiveMailWidget::slotAddItem()
{
  AddArchiveMailDialog *dialog = new AddArchiveMailDialog(this);
  if( dialog->exec() ) {
    //FIXME
   ArchiveMailItem *item = new ArchiveMailItem(i18n("foo"), mWidget->listWidget);
   //TODO
  }
  delete dialog;
}

#include "archivemaildialog.moc"

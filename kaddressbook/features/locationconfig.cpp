/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kbuttonbox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>

#include "locationconfig.h"

LocationConfigWidget::LocationConfigWidget( ViewManager *vm, QWidget *parent,
                                            const char *name )
  : ConfigureWidget( vm, parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 3, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );
  layout->setColStretch( 1, 1 );

  mListView = new KListView( this );
  mListView->addColumn( i18n( "Name" ) );
  mListView->addColumn( i18n( "URL" ) );
  mListView->setAllColumnsShowFocus( true );
  layout->addMultiCellWidget( mListView, 0, 0, 0, 1 );

  KButtonBox *bbox = new KButtonBox( this, Qt::Vertical );
  mAddButton = bbox->addButton( i18n( "Add" ), this, SLOT( add() ) );
  mEditButton = bbox->addButton( i18n( "Edit" ), this, SLOT( edit() ) );
  mRemoveButton = bbox->addButton( i18n( "Remove" ), this, SLOT( remove() ) );
  bbox->layout();
  layout->addWidget( bbox, 0, 2 );

  QLabel *label = new QLabel( i18n( "Name:" ), this );
  layout->addWidget( label, 1, 0 );

  mNameEdit = new KLineEdit( this );
  label->setBuddy( mNameEdit );
  layout->addMultiCellWidget( mNameEdit, 1, 1, 1, 2 );

  label = new QLabel( i18n( "URL:" ), this );
  layout->addWidget( label, 2, 0 );

  mURLEdit = new KLineEdit( this );
  label->setBuddy( mURLEdit );
  layout->addMultiCellWidget( mURLEdit, 2, 2, 1, 2 );
  QToolTip::add( mURLEdit, i18n( "<ul> <li>%s: Street</li>"
                                 "<li>%r: Region</li>"
                                 "<li>%l: Location</li>"
                                 "<li>%z: Zip Code</li>"
                                 "<li>%c: Country ISO Code</li> </ul>" ) );

  resize( 500, 300 );
}

LocationConfigWidget::~LocationConfigWidget()
{
}

void LocationConfigWidget::restoreSettings( KConfig *cfg )
{
  mListView->clear();

  QMap<QString, QString> map = cfg->entryMap( cfg->group() );

  if ( map.empty() ) // add a default
    map.insert( "map24", "http://map24.de/map24/index.php3?street0=%s&zip0=%z&city0=%l&country0=%c&force_maptype=RELOAD&gcf=1" );

  QMap<QString, QString>::Iterator it;
  for ( it = map.begin(); it != map.end(); ++it )
    new QListViewItem( mListView, it.key(), it.data() );
}

void LocationConfigWidget::saveSettings( KConfig *cfg )
{
  QString group = cfg->group();
  cfg->deleteGroup( group );
  cfg->setGroup( group );

  QListViewItem *item;
  for ( item = mListView->firstChild(); item; item = item->itemBelow() )
    cfg->writeEntry( item->text( 0 ), item->text( 1 ) );
}

void LocationConfigWidget::add()
{
  new QListViewItem( mListView, mNameEdit->text(), mURLEdit->text() );
}

void LocationConfigWidget::remove()
{
  QListViewItem *item = mListView->currentItem();
  mListView->takeItem( item );
  delete item;
}

void LocationConfigWidget::edit()
{
  QListViewItem *item = mListView->currentItem();
  if ( !item )
    return;

  mNameEdit->setText( item->text( 0 ) );
  mURLEdit->setText( item->text( 1 ) );
}

void LocationConfigWidget::selectionChanged( QListViewItem *item )
{
  bool state = ( item != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

void LocationConfigWidget::inputChanged( const QString &text )
{
  mAddButton->setEnabled( !text.isEmpty() );
}

#include "locationconfig.moc"

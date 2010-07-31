/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <kdialog.h>
#include <klocale.h>
#include <kwidgetlist.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>

#include "memberinfo.h"

#include "multiconflictdialog.h"

class ChangeItem : public KWidgetListItem
{
  public:
    ChangeItem( KWidgetList *parent, const QSync::SyncChange &change )
      : KWidgetListItem( parent ),
        mChange( change )
    {
      TQGridLayout *layout = new TQGridLayout( this, 2, 1, KDialog::marginHint(), KDialog::spacingHint() );

      MemberInfo mi( change.member() );
      layout->addWidget( new TQLabel( mi.name(), this ), 0, 0 );

      TQString type;
      switch ( change.changeType() ) {
        case QSync::SyncChange::UnknownChange:
          type = i18n( "Unknown" );
          break;
        case QSync::SyncChange::AddedChange:
          type = i18n( "Added" );
          break;
        case QSync::SyncChange::DeletedChange:
          type = i18n( "Deleted" );
          break;
        case QSync::SyncChange::ModifiedChange:
          type = i18n( "Modified" );
          break;
        case QSync::SyncChange::UnmodifiedChange:
        default:
          type = i18n( "Unmodified" );
          break;
      }

      layout->addWidget( new TQLabel( type, this ), 1, 0 );
    }

    QSync::SyncChange change() const { return mChange; }

  private:
    QSync::SyncChange mChange;
};

MultiConflictDialog::MultiConflictDialog( QSync::SyncMapping &mapping, TQWidget *parent )
  : ConflictDialog( mapping, parent )
{
  initGUI();

  for ( int i = 0; i < mMapping.changesCount(); ++i ) {
    QSync::SyncChange change = mMapping.changeAt( i );
    if ( change.isValid() ) {
      ChangeItem *item = new ChangeItem( mWidgetList, change );
      mWidgetList->appendItem( item );
    }
  }

  mWidgetList->setFocus();
}

MultiConflictDialog::~MultiConflictDialog()
{
}

void MultiConflictDialog::useSelectedChange()
{
  ChangeItem *item = static_cast<ChangeItem*>( mWidgetList->selectedItem() );
  if ( !item )
    return;

  mMapping.solve( item->change() );

  accept();
}

void MultiConflictDialog::duplicateChange()
{
  mMapping.duplicate();

  accept();
}

void MultiConflictDialog::ignoreChange()
{
  mMapping.ignore();

  accept();
}

void MultiConflictDialog::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( this, 3, 3, KDialog::marginHint(), KDialog::spacingHint() );

  layout->addMultiCellWidget( new TQLabel( i18n( "A conflict has appeared, please solve it manually." ), this ), 0, 0, 0, 2 );

  mWidgetList = new KWidgetList( this );
  layout->addMultiCellWidget( mWidgetList, 1, 1, 0, 2 );

  TQPushButton *button = new TQPushButton( i18n( "Use Selected Item" ), this );
  connect( button, TQT_SIGNAL( clicked() ), TQT_SLOT( useSelectedChange() ) );
  layout->addWidget( button, 2, 0 );

  button = new TQPushButton( i18n( "Duplicate Items" ), this );
  connect( button, TQT_SIGNAL( clicked() ), TQT_SLOT( duplicateChange() ) );
  layout->addWidget( button, 2, 1 );

  button = new TQPushButton( i18n( "Ignore Conflict" ), this );
  connect( button, TQT_SIGNAL( clicked() ), TQT_SLOT( ignoreChange() ) );
  layout->addWidget( button, 2, 2 );
}

#include "multiconflictdialog.moc"

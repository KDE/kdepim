/*
    This file is part of KAddressBook.
    Copyright (c) 2007 Klaralvdalens Datakonsult AB <frank@kdab.net>

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
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "distributionlistngwidget.h"
#include "interfaces/core.h"
#include "searchmanager.h"

#include <libkdepim/distributionlist.h>
#include <libkdepim/kvcarddrag.h>

#include <kabc/vcardconverter.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <tqevent.h>
#include <tqguardedptr.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpoint.h>
#include <tqtimer.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>

KAB::DistributionListNg::ListBox::ListBox( TQWidget* parent ) : KListBox( parent )
{
    setAcceptDrops( true );
}

void KAB::DistributionListNg::ListBox::dragMoveEvent( TQDragMoveEvent *event )
{
    TQListBoxItem *item = itemAt( event->pos() );
    if ( !item ) {
        event->ignore();
    }
    else {
        event->accept( itemRect( item ) );
    }
}

void KAB::DistributionListNg::ListBox::dragEnterEvent( TQDragEnterEvent *event )
{
    KListBox::dragEnterEvent( event );
}

void KAB::DistributionListNg::ListBox::dropEvent( TQDropEvent *event )
{
    TQListBoxItem *item = itemAt( event->pos() );
    if ( !item || index( item ) == 0 )
        return;

    KABC::Addressee::List list;
    if ( !KVCardDrag::decode( event, list ) )
        return;

    emit dropped( item->text(), list );
}

namespace KAB {
namespace DistributionListNg {

class Factory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, TQWidget *parent, const char *name )
    {
      return new KAB::DistributionListNg::MainWidget( core, parent, name );
    }

    TQString identifier() const
    {
      return "distribution_list_editor";
    }
};

}
}

extern "C" {
  void *init_libkaddrbk_distributionlistng()
  {
    return ( new KAB::DistributionListNg::Factory );
  }
}

TQString KAB::DistributionListNg::MainWidget::title() const
{
    return i18n( "Distribution List Editor NG" );
}

TQString KAB::DistributionListNg::MainWidget::identifier() const
{
    return "distribution_list_editor_ng";
}

KAB::DistributionListNg::MainWidget::MainWidget( KAB::Core *core, TQWidget *parent, const char *name ) : KAB::ExtensionWidget( core, parent, name )
{
    TQVBoxLayout *layout = new TQVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    TQHBoxLayout *buttonLayout = new TQHBoxLayout();
    layout->addLayout( buttonLayout );

    TQLabel *label = new TQLabel( this );
    label->setText( i18n( "Distribution Lists" ) );
    buttonLayout->addWidget( label );
    buttonLayout->addStretch( 1 );

    mAddButton = new TQPushButton( this );
    mAddButton->setIconSet( SmallIconSet( "add" ) );
    TQToolTip::add( mAddButton, i18n( "Add distribution list" ) );
    connect( mAddButton, TQT_SIGNAL(clicked()), core, TQT_SLOT(newDistributionList()) );
    buttonLayout->addWidget( mAddButton );

    mEditButton = new TQPushButton( this );
    mEditButton->setIconSet( SmallIconSet( "edit" ) );
    TQToolTip::add( mEditButton, i18n( "Edit distribution list" ) );
    connect( mEditButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(editSelectedDistributionList()) );
    buttonLayout->addWidget( mEditButton );

    mRemoveButton = new TQPushButton( this );
    mRemoveButton->setIconSet( SmallIconSet( "remove" ) );
    TQToolTip::add( mRemoveButton, i18n( "Remove distribution list" ) );
    connect( mRemoveButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(deleteSelectedDistributionList()) );
    buttonLayout->addWidget( mRemoveButton );

    mListBox = new ListBox( this );
    connect( mListBox, TQT_SIGNAL( contextMenuRequested( TQListBoxItem*, const TQPoint& ) ),
             this, TQT_SLOT( contextMenuRequested( TQListBoxItem*, const TQPoint& ) ) );
    connect( mListBox, TQT_SIGNAL( dropped( const TQString &, const KABC::Addressee::List & ) ),
             this, TQT_SLOT( contactsDropped( const TQString &, const KABC::Addressee::List & ) ) );
    connect( mListBox, TQT_SIGNAL( highlighted( int ) ),
             this, TQT_SLOT( itemSelected( int ) ) );
    connect( mListBox, TQT_SIGNAL(doubleClicked(TQListBoxItem*)), TQT_SLOT(editSelectedDistributionList()) );
    layout->addWidget( mListBox );

    connect( core, TQT_SIGNAL( contactsUpdated() ),
             this, TQT_SLOT( updateEntries() ) );
    connect( core->addressBook(), TQT_SIGNAL( addressBookChanged( AddressBook* ) ),
             this, TQT_SLOT( updateEntries() ) );

    // When contacts are changed, update both distr list combo and contents of displayed distr list
    connect( core, TQT_SIGNAL( contactsUpdated() ),
             this, TQT_SLOT( updateEntries() ) );

    TQTimer::singleShot( 0, this, TQT_SLOT( updateEntries() ) );
}

void KAB::DistributionListNg::MainWidget::contextMenuRequested( TQListBoxItem *item, const TQPoint &point )
{
    TQGuardedPtr<KPopupMenu> menu = new KPopupMenu( this );
    menu->insertItem( i18n( "New Distribution List..." ), core(), TQT_SLOT( newDistributionList() ) );
    if ( item && ( item->text() !=i18n( "All Contacts" ) ) )
    {
        menu->insertItem( i18n( "Edit..." ), this, TQT_SLOT( editSelectedDistributionList() ) );
        menu->insertItem( i18n( "Delete" ), this, TQT_SLOT( deleteSelectedDistributionList() ) );
    }
    menu->exec( point );
    delete menu;
}

void KAB::DistributionListNg::MainWidget::editSelectedDistributionList()
{
    const TQListBoxItem* const item = mListBox->selectedItem();
    if ( !item )
        return;
    core()->editDistributionList( item->text() );
}

void KAB::DistributionListNg::MainWidget::deleteSelectedDistributionList()
{
    const TQListBoxItem* const item = mListBox->selectedItem();
    const TQString name = item ? item->text() : TQString();
    if ( name.isNull() )
        return;
    const KPIM::DistributionList list = KPIM::DistributionList::findByName(
    core()->addressBook(), name );
    if ( list.isEmpty() )
        return;
    core()->deleteDistributionLists( name );
}

void KAB::DistributionListNg::MainWidget::contactsDropped( const TQString &listName, const KABC::Addressee::List &addressees )
{
    if ( addressees.isEmpty() )
        return;

    KPIM::DistributionList list = KPIM::DistributionList::findByName(
    core()->addressBook(), listName );
    if ( list.isEmpty() ) // not found [should be impossible]
        return;

    for ( KABC::Addressee::List::ConstIterator it = addressees.begin(); it != addressees.end(); ++it ) {
        list.insertEntry( *it );
    }

    core()->addressBook()->insertAddressee( list );
    changed( list );
}

void KAB::DistributionListNg::MainWidget::changed( const KABC::Addressee& dist )
{
    emit modified( KABC::Addressee::List() << dist );
}

void KAB::DistributionListNg::MainWidget::updateEntries()
{
    const bool hadSelection = mListBox->selectedItem() != 0;
    const TQStringList newEntries = core()->distributionListNames();
    if ( !mCurrentEntries.isEmpty() && newEntries == mCurrentEntries )
        return;
    mCurrentEntries = newEntries;
    mListBox->clear();
    mListBox->insertItem( i18n( "All Contacts" ), 0 );
    mListBox->insertStringList( mCurrentEntries );
    if ( !hadSelection )
        mListBox->setSelected( 0, true );
}

void KAB::DistributionListNg::MainWidget::itemSelected( int index )
{
    core()->setSelectedDistributionList( index == 0 ? TQString() : mListBox->item( index )->text()  );
    mEditButton->setEnabled( index > 0 );
    mRemoveButton->setEnabled( index > 0 );
}

#include "distributionlistngwidget.moc"

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

#include <qevent.h>
#include <qguardedptr.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpoint.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtooltip.h>

KAB::DistributionListNg::ListBox::ListBox( QWidget* parent ) : KListBox( parent )
{
    setAcceptDrops( true );
}

void KAB::DistributionListNg::ListBox::dragMoveEvent( QDragMoveEvent *event )
{
    QListBoxItem *item = itemAt( event->pos() );
    if ( !item ) {
        event->ignore();
    }
    else {
        event->accept( itemRect( item ) );
    }
}

void KAB::DistributionListNg::ListBox::dragEnterEvent( QDragEnterEvent *event )
{
    KListBox::dragEnterEvent( event );
}

void KAB::DistributionListNg::ListBox::dropEvent( QDropEvent *event )
{
    QListBoxItem *item = itemAt( event->pos() );
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
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new KAB::DistributionListNg::MainWidget( core, parent, name );
    }

    QString identifier() const
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

QString KAB::DistributionListNg::MainWidget::title() const
{
    return i18n( "Distribution List Editor NG" );
}

QString KAB::DistributionListNg::MainWidget::identifier() const
{
    return "distribution_list_editor_ng";
}

KAB::DistributionListNg::MainWidget::MainWidget( KAB::Core *core, QWidget *parent, const char *name ) : KAB::ExtensionWidget( core, parent, name )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    layout->addLayout( buttonLayout );

    QLabel *label = new QLabel( this );
    label->setText( i18n( "Distribution Lists" ) );
    buttonLayout->addWidget( label );
    buttonLayout->addStretch( 1 );

    mAddButton = new QPushButton( this );
    mAddButton->setIconSet( SmallIconSet( "add" ) );
    QToolTip::add( mAddButton, i18n( "Add distribution list" ) );
    connect( mAddButton, SIGNAL(clicked()), core, SLOT(newDistributionList()) );
    buttonLayout->addWidget( mAddButton );

    mEditButton = new QPushButton( this );
    mEditButton->setIconSet( SmallIconSet( "edit" ) );
    QToolTip::add( mEditButton, i18n( "Edit distribution list" ) );
    connect( mEditButton, SIGNAL(clicked()), this, SLOT(editSelectedDistributionList()) );
    buttonLayout->addWidget( mEditButton );

    mRemoveButton = new QPushButton( this );
    mRemoveButton->setIconSet( SmallIconSet( "remove" ) );
    QToolTip::add( mRemoveButton, i18n( "Remove distribution list" ) );
    connect( mRemoveButton, SIGNAL(clicked()), this, SLOT(deleteSelectedDistributionList()) );
    buttonLayout->addWidget( mRemoveButton );

    mListBox = new ListBox( this );
    connect( mListBox, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ),
             this, SLOT( contextMenuRequested( QListBoxItem*, const QPoint& ) ) );
    connect( mListBox, SIGNAL( dropped( const QString &, const KABC::Addressee::List & ) ),
             this, SLOT( contactsDropped( const QString &, const KABC::Addressee::List & ) ) );
    connect( mListBox, SIGNAL( highlighted( int ) ),
             this, SLOT( itemSelected( int ) ) );
    connect( mListBox, SIGNAL(doubleClicked(QListBoxItem*)), SLOT(editSelectedDistributionList()) );
    layout->addWidget( mListBox );

    connect( core, SIGNAL( contactsUpdated() ),
             this, SLOT( updateEntries() ) );
    connect( core->addressBook(), SIGNAL( addressBookChanged( AddressBook* ) ),
             this, SLOT( updateEntries() ) );

    // When contacts are changed, update both distr list combo and contents of displayed distr list
    connect( core, SIGNAL( contactsUpdated() ),
             this, SLOT( updateEntries() ) );

    QTimer::singleShot( 0, this, SLOT( updateEntries() ) );
}

void KAB::DistributionListNg::MainWidget::contextMenuRequested( QListBoxItem *item, const QPoint &point )
{
    QGuardedPtr<KPopupMenu> menu = new KPopupMenu( this );
    menu->insertItem( i18n( "New Distribution List..." ), core(), SLOT( newDistributionList() ) );
    if ( item && ( item->text() !=i18n( "All Contacts" ) ) )
    {
        menu->insertItem( i18n( "Edit..." ), this, SLOT( editSelectedDistributionList() ) );
        menu->insertItem( i18n( "Delete" ), this, SLOT( deleteSelectedDistributionList() ) );
    }
    menu->exec( point );
    delete menu;
}

void KAB::DistributionListNg::MainWidget::editSelectedDistributionList()
{
    const QListBoxItem* const item = mListBox->selectedItem();
    if ( !item )
        return;
    core()->editDistributionList( item->text() );
}

void KAB::DistributionListNg::MainWidget::deleteSelectedDistributionList()
{
    const QListBoxItem* const item = mListBox->selectedItem();
    const QString name = item ? item->text() : QString();
    if ( name.isNull() )
        return;
    const KPIM::DistributionList list = KPIM::DistributionList::findByName(
    core()->addressBook(), name );
    if ( list.isEmpty() )
        return;
    core()->deleteDistributionLists( name );
}

void KAB::DistributionListNg::MainWidget::contactsDropped( const QString &listName, const KABC::Addressee::List &addressees )
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
    const QStringList newEntries = core()->distributionListNames();
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
    core()->setSelectedDistributionList( index == 0 ? QString() : mListBox->item( index )->text()  );
    mEditButton->setEnabled( index > 0 );
    mRemoveButton->setEnabled( index > 0 );
}

#include "distributionlistngwidget.moc"

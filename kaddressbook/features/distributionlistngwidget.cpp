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
#include <klocale.h>
#include <KMenu>

#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QListWidgetItem>
#include <qpoint.h>
#include <QPointer>
#include <QTimer>

KAB::DistributionListNg::ListBox::ListBox( QWidget* parent ) : QListWidget( parent )
{
    setAcceptDrops( true );
    setSelectionMode( QAbstractItemView::SingleSelection );
}

void KAB::DistributionListNg::ListBox::dragMoveEvent( QDragMoveEvent *event )
{
    QListWidgetItem *item = itemAt( event->pos() );
    if ( !item ) {
        event->ignore();
    }
    else {
        event->accept( visualItemRect( item ) );
    }   
} 

void KAB::DistributionListNg::ListBox::dragEnterEvent( QDragEnterEvent *event )
{
    QListWidget::dragEnterEvent( event );
}

void KAB::DistributionListNg::ListBox::dropEvent( QDropEvent *event )
{
    QListWidgetItem *item = itemAt( event->pos() );
    if ( !item || item == this->item( 0 ) )
        return;

    KABC::Addressee::List addrs;
    if ( !KPIM::KVCardDrag::fromMimeData( event->mimeData(), addrs ) ) 
        return;

    emit dropped( item->text(), addrs );
}

namespace KAB {
namespace DistributionListNg {

class Factory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent )
    {
      return new KAB::DistributionListNg::MainWidget( core, parent );
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

KAB::DistributionListNg::MainWidget::MainWidget( KAB::Core *core, QWidget *parent ) : KAB::ExtensionWidget( core, parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    QLabel *label = new QLabel;
    label->setText( i18n( "Distribution Lists" ) );
    layout->addWidget( label );

    mListBox = new ListBox;
    layout->addWidget( mListBox );

    connect( mListBox, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ), 
             this, SLOT( contextMenuRequested( QListBoxItem*, const QPoint& ) ) );
    connect( mListBox, SIGNAL( dropped( const QString &, const KABC::Addressee::List & ) ), 
             this, SLOT( contactsDropped( const QString &, const KABC::Addressee::List & ) ) );
    connect( mListBox, SIGNAL( highlighted( int ) ), 
             this, SLOT( itemSelected( int ) ) );


    connect( core, SIGNAL( contactsUpdated() ),
             this, SLOT( updateEntries() ) );
    connect( core->addressBook(), SIGNAL( addressBookChanged( AddressBook* ) ),
             this, SLOT( updateEntries() ) );

    // When contacts are changed, update both distr list combo and contents of displayed distr list
    connect( core, SIGNAL( contactsUpdated() ),
             this, SLOT( updateEntries() ) );

    QTimer::singleShot( 0, this, SLOT( updateEntries() ) );
}

#warning port! PENDING(KDAB)
#if 0
void KAB::DistributionListNg::MainWidget::contextMenuRequested( QListWidgetItem *item, const QPoint &point )
{
    QPointer<KMenu> menu = new KMenu( this );
    menu->insertItem( i18n( "New Distribution List..." ), core(), SLOT( newDistributionList() ) ); 
    if ( item )
    {
        menu->insertItem( i18n( "Edit..." ), this, SLOT( editSelectedDistributionList() ) ); 
        menu->insertItem( i18n( "Delete" ), this, SLOT( deleteSelectedDistributionList() ) ); 
    }
    menu->exec( point );
    delete menu;
}

#endif 

void KAB::DistributionListNg::MainWidget::editSelectedDistributionList()
{
    const QList<QListWidgetItem*> items = mListBox->selectedItems();
    if ( items.isEmpty() )
        return;
    core()->editDistributionList( items.first()->text() );
}

void KAB::DistributionListNg::MainWidget::deleteSelectedDistributionList()
{
    const QList<QListWidgetItem*> items = mListBox->selectedItems();

    const QString name = items.isEmpty() ? items.first()->text() : QString();
    if ( name.isNull() )
        return;
    const KPIM::DistributionList list = KPIM::DistributionList::findByName(
    core()->addressBook(), name );
    if ( list.isEmpty() )
        return;
    core()->deleteDistributionLists( QStringList( name ) );
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
    const bool hadSelection = !mListBox->selectedItems().isEmpty();
    const QStringList newEntries = core()->distributionListNames();
    if ( !newEntries.isEmpty() && newEntries == mCurrentEntries )
        return;
    mCurrentEntries = newEntries;
    mListBox->clear();
    mListBox->insertItem( 0, i18n( "All Contacts" ) );
    mListBox->addItems( mCurrentEntries );
    if ( !hadSelection )
        mListBox->item( 0 )->setSelected( true );
}

void KAB::DistributionListNg::MainWidget::itemSelected( int index )
{
    core()->setSelectedDistributionList( index == 0 ? QString() : mListBox->item( index )->text()  );
}

#include "distributionlistngwidget.moc"

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

#include <QtCore/QEvent>
#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidgetItem>
#include <QtGui/QToolButton>

#include <kabc/distributionlist.h>
#include <kabc/vcardconverter.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmenu.h>
#include <libkdepim/distributionlist.h>
#include <libkdepim/kvcarddrag.h>

#include "interfaces/core.h"
#include "searchmanager.h"

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
    event->accept();
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

K_EXPORT_PLUGIN(KAB::DistributionListNg::Factory)

QString KAB::DistributionListNg::MainWidget::title() const
{
    return i18n( "Distribution List Editor" );
}

QString KAB::DistributionListNg::MainWidget::identifier() const
{
    return "distribution_list_editor_ng";
}

KAB::DistributionListNg::MainWidget::MainWidget( KAB::Core *core, QWidget *parent ) : KAB::ExtensionWidget( core, parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    layout->addLayout( buttonLayout );

    QLabel *label = new QLabel;
    label->setText( i18n( "Distribution Lists" ) );
    buttonLayout->addWidget( label );
    buttonLayout->addStretch( 1 );

    mAddButton = new QToolButton( this );
    mAddButton->setIcon( KIcon( "list-add" ) );
    mAddButton->setToolTip( i18n( "Add distribution list" ) );
    connect( mAddButton, SIGNAL(clicked()), core, SLOT(newDistributionList()) );
    buttonLayout->addWidget( mAddButton );

    mEditButton = new QToolButton( this );
    mEditButton->setIcon( KIcon( "document-properties" ) );
    mEditButton->setToolTip( i18n( "Edit distribution list" ) );
    mEditButton->setEnabled( false );
    connect( mEditButton, SIGNAL(clicked()), this, SLOT(editSelectedDistributionList()) );
    buttonLayout->addWidget( mEditButton );

    mRemoveButton = new QToolButton( this );
    mRemoveButton->setIcon( KIcon( "edit-delete" ) );
    mRemoveButton->setToolTip( i18n( "Remove distribution list" ) );
    mRemoveButton->setEnabled( false );
    connect( mRemoveButton, SIGNAL(clicked()), this, SLOT(deleteSelectedDistributionList()) );
    buttonLayout->addWidget( mRemoveButton );

    mListBox = new ListBox;
    mListBox->setContextMenuPolicy( Qt::CustomContextMenu );
    layout->addWidget( mListBox );

    connect( mListBox, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( contextMenuRequested( const QPoint& ) ) );
    connect( mListBox, SIGNAL( dropped( const QString &, const KABC::Addressee::List & ) ),
             this, SLOT( contactsDropped( const QString &, const KABC::Addressee::List & ) ) );
    connect( mListBox, SIGNAL( currentRowChanged( int ) ),
             this, SLOT( itemSelected( int ) ) );
    connect( mListBox, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             SLOT(editSelectedDistributionList()) );


    connect( core->addressBook(), SIGNAL( addressBookChanged( AddressBook* ) ),
             this, SLOT( updateEntries() ) );

    // When contacts are changed, update both distr list combo and contents of displayed distr list
    connect( core, SIGNAL( contactsUpdated() ),
             this, SLOT( updateEntries() ) );

    QTimer::singleShot( 0, this, SLOT( updateEntries() ) );
}

void KAB::DistributionListNg::MainWidget::contextMenuRequested( const QPoint &point )
{
    QListWidgetItem *item = mListBox->itemAt( point );
    QPointer<KMenu> menu = new KMenu( this );
    menu->addAction( KIcon( "list-add" ), i18n( "New Distribution List..." ), core(), SLOT( newDistributionList() ) );
    if ( item && mListBox->item( 0 ) != item )
    {
        menu->addAction( KIcon( "document-properties" ), i18n( "Edit..." ), this, SLOT( editSelectedDistributionList() ) );
        menu->addAction( KIcon( "edit-delete" ), i18n( "Delete" ), this, SLOT( deleteSelectedDistributionList() ) );
    }
    menu->exec( mListBox->mapToGlobal( point ) );
    delete menu;
}

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
    const QString name = items.isEmpty() ? QString() : items.first()->text();
    kDebug() << "name=" << name;
    if ( name.isEmpty() )
        return;

    const KABC::DistributionList *list =
        core()->addressBook()->findDistributionListByName( name );
    if ( !list )
        return;

    core()->deleteDistributionLists( QStringList( name ) );
}

void KAB::DistributionListNg::MainWidget::contactsDropped( const QString &listName, const KABC::Addressee::List &addressees )
{
    if ( addressees.isEmpty() )
        return;

    KABC::DistributionList *list =
        core()->addressBook()->findDistributionListByName( listName );
    if ( !list ) // not found [should be impossible]
        return;

    for ( KABC::Addressee::List::ConstIterator it = addressees.begin(); it != addressees.end(); ++it ) {
        list->insertEntry( *it );
    }

    changed( list );
}

void KAB::DistributionListNg::MainWidget::changed( const KABC::DistributionList* dist )
{
    emit modified( dist );
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
    QString selectedList;
    if ( index != 0 ) {
      QListWidgetItem *item = mListBox->item( index );
      if ( item )
        selectedList = item->text();
    }

    core()->setSelectedDistributionList( selectedList );
    mEditButton->setEnabled( index > 0 );
    mRemoveButton->setEnabled( index > 0 );
}

#include "distributionlistngwidget.moc"

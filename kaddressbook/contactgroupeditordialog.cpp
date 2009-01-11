/*
    This file is part of KContactManager.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "contactgroupeditordialog.h"

#include "kabc/contactgroupeditor.h"

#include "collectioncombobox.h"
#include <akonadi/item.h>

#include <klocale.h>

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

ContactGroupEditorDialog::ContactGroupEditorDialog( Mode mode, QAbstractItemModel *collectionModel, QWidget *parent )
  : KDialog( parent )
{
  setCaption( mode == CreateMode ? i18n( "New Contact Group" ) : i18n( "Edit Contact Group" ) );
  setButtons( Ok | Cancel );

  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  QGridLayout *layout = new QGridLayout( mainWidget );

  mEditor = new Akonadi::ContactGroupEditor( mode == CreateMode ?
                                             Akonadi::ContactGroupEditor::CreateMode : Akonadi::ContactGroupEditor::EditMode, this );

  if ( mode == CreateMode ) {
     QLabel *label = new QLabel( i18n( "Add to:" ), mainWidget );
     KABC::CollectionComboBox *box = new KABC::CollectionComboBox( mainWidget );
     if ( collectionModel )
       box->setModel( collectionModel );

     layout->addWidget( label, 0, 0 );
     layout->addWidget( box, 0, 1 );

     connect( box, SIGNAL( selectionChanged( const Akonadi::Collection& ) ),
              mEditor, SLOT( setDefaultCollection( const Akonadi::Collection& ) ) );

     mEditor->setDefaultCollection( box->selectedCollection() );
  }

  layout->addWidget( mEditor, 1, 0, 1, 2 );
  layout->setColumnStretch( 1, 1 );

  connect( mEditor, SIGNAL( contactGroupStored( const Akonadi::Item& ) ),
           this, SIGNAL( contactGroupStored( const Akonadi::Item& ) ) );

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOkClicked() ) );
  connect( this, SIGNAL( cancelClicked() ), this, SLOT( slotCancelClicked() ) );

  setInitialSize( QSize( 420, 120 ) );
}

ContactGroupEditorDialog::~ContactGroupEditorDialog()
{
}

void ContactGroupEditorDialog::setContactGroup( const Akonadi::Item &group )
{
  mEditor->loadContactGroup( group );
}

void ContactGroupEditorDialog::slotOkClicked()
{
  mEditor->saveContactGroup();

  accept();
}

void ContactGroupEditorDialog::slotCancelClicked()
{
  reject();
}

#include "contactgroupeditordialog.moc"

/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "entitytreewidget.h"

#include <QTreeView>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>


#include <akonadi/changerecorder.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

static const QString predefinedMimeTypes[] =
{
  "",
  "message/rfc822",
  "text/directory",
  "text/calendar",
  "message/rfc822,text/directory"
};

static const QString predefinedUserVisibleMimeTypes[] =
{
  "All",
  "Email",
  "Addressees",
  "Events",
  "Email and Addressees"
};

EntityTreeWidget::EntityTreeWidget( QWidget* parent )
  : QWidget( parent ),
    m_treeView( new QTreeView ),
    m_typeComboBox( new QComboBox ),
    m_typeLineEdit( new QLineEdit ),
    m_changeRecorder( new ChangeRecorder( this ) )
{
  for ( uint i = 0; i < sizeof predefinedMimeTypes / sizeof *predefinedMimeTypes; ++i )
    m_typeComboBox->addItem(predefinedUserVisibleMimeTypes[i], predefinedMimeTypes[i]);

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( m_typeComboBox );
  layout->addWidget( m_typeLineEdit );
  layout->addWidget( m_treeView );

  m_changeRecorder->setCollectionMonitored( Collection::root() );
  m_changeRecorder->fetchCollection( true );
  m_changeRecorder->setAllMonitored( true );
  m_changeRecorder->itemFetchScope().fetchFullPayload( true );
  m_changeRecorder->itemFetchScope().fetchAllAttributes( true );
  m_etm = new EntityTreeModel( m_changeRecorder, this );

  m_treeView->setModel( m_etm );

  connect(m_typeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(mimeTypesChoiceChanged(int)));
  connect(m_typeLineEdit, SIGNAL(textChanged(QString)), SLOT(mimeTypesChanged(QString)));
}

void EntityTreeWidget::mimeTypesChoiceChanged( int index )
{
  m_typeLineEdit->setText( m_typeComboBox->itemData( index ).toString() );
}

void EntityTreeWidget::mimeTypesChanged( const QString& mimetypeList )
{
  QStringList list = mimetypeList.isEmpty() ? QStringList() : mimetypeList.split(",");

  foreach ( const QString mimetype, m_changeRecorder->mimeTypesMonitored() )
    if ( !list.contains( mimetype ) )
      m_changeRecorder->setMimeTypeMonitored( mimetype, false );

  foreach ( const QString mimetype, list )
    m_changeRecorder->setMimeTypeMonitored( mimetype, true );
}

QTreeView* EntityTreeWidget::view() const
{
  return m_treeView;
}



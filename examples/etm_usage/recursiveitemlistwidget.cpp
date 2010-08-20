/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

// READ THE README FILE

#include "recursiveitemlistwidget.h"

#include <QHBoxLayout>
#include <QListView>

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/changerecorder.h>

#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/entitylistview.h>
#include <akonadi/entitytreeview.h>
#include <KABC/Addressee>
#include <Akonadi/ItemFetchScope>

RecursiveItemListWidget::RecursiveItemListWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{

  Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder(this);
  changeRecorder->setAllMonitored( true );
  changeRecorder->setMimeTypeMonitored( KABC::Addressee::mimeType() );
  changeRecorder->itemFetchScope().fetchFullPayload( true );
  m_etm = new Akonadi::ContactsTreeModel(changeRecorder, this);

  m_etm->setCollectionFetchStrategy( Akonadi::EntityTreeModel::InvisibleCollectionFetch );

  Akonadi::EntityMimeTypeFilterModel *list = new Akonadi::EntityMimeTypeFilterModel( this );
  list->setSourceModel(m_etm);
  list->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );

  m_listView = new Akonadi::EntityTreeView;
  m_listView->setModel( list );
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget( m_listView );

}

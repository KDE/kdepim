/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "itemviewerwidget.h"

#include <QItemSelectionModel>
#include <QItemSelectionRange>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <akonadi/entitytreemodel.h>

#include "messageviewer/viewer.h"
#include "akonadi/contact/contactviewer.h"

using namespace Akonadi;

ItemViewerWidget::ItemViewerWidget( QItemSelectionModel *itemSelectionModel, QWidget* parent, Qt::WindowFlags f )
  : QWidget( parent, f ), m_itemSelectionModel( itemSelectionModel ), m_widgetStack(new QStackedWidget(this))
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(m_widgetStack);

  m_widgetStack->addWidget(new QWidget(this));
  m_mailViewer = new MessageViewer::Viewer(this);
  m_contactViewer = new Akonadi::ContactViewer(this);
  m_widgetStack->addWidget(m_mailViewer);
  m_widgetStack->addWidget(m_contactViewer);

  connect(itemSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
}

void ItemViewerWidget::selectionChanged( const QItemSelection selected, const QItemSelection& deselected )
{
  Q_UNUSED(deselected);
  if(selected.indexes().size() != 1)
    return;
  QModelIndex selectedIndex = selected.indexes().first();
  QString mimeType = selectedIndex.data(EntityTreeModel::MimeTypeRole).toString();
  Akonadi::Item item = selectedIndex.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (mimeType == "message/rfc822")
  {
    m_widgetStack->setCurrentIndex( 1 );
    m_mailViewer->setMessageItem( item, MessageViewer::Viewer::Force );
    return;
  }
  if (mimeType == "text/directory")
  {
    m_widgetStack->setCurrentIndex( 2 );
    m_contactViewer->setItem( item );
    return;
  }
  m_widgetStack->setCurrentIndex( 0 );
}

#include "itemviewerwidget.moc"


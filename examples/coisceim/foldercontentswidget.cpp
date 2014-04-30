/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#include "foldercontentswidget.h"

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QPushButton>

#include <KLocale>

#include <AkonadiWidgets/CollectionDialog>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/EntityTreeView>

#include <KMime/kmime_message.h>

#include <KCalCore/Todo>

#include "trip.h"
#include "note.h"

using namespace Akonadi;

FolderContentsWidget::FolderContentsWidget(Trip *trip, int role, const QString &type, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_type(type), m_trip(trip), m_role(role)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  QPushButton *confButton = new QPushButton(type);
  layout->addWidget(confButton);
  connect(confButton, SIGNAL(clicked()), SLOT(configure()));

  m_view = new EntityTreeView;
  layout->addWidget(m_view);

  switch(role) {
  case Trip::MailCollectionRole:
    m_view->setModel(m_trip->mailModel());
    break;
  case Trip::TodoCollectionRole:
    m_view->setModel(m_trip->todoModel());
    break;
  case Trip::NotesCollectionRole:
    m_view->setModel(m_trip->notesModel());
    break;
  }
}

QItemSelectionModel* FolderContentsWidget::selectionModel() const
{
  return m_view->selectionModel();
}

static QString getMimeType(int role)
{
  switch(role) {
  case Trip::MailCollectionRole: return KMime::Message::mimeType();
  case Trip::TodoCollectionRole: return KCalCore::Todo::todoMimeType();
  case Trip::NotesCollectionRole: return Akonotes::Note::mimeType();
  }
  return QString();
}

void FolderContentsWidget::configure()
{
  CollectionDialog dlg;

  dlg.setMimeTypeFilter( QStringList() << getMimeType(m_role) );
  dlg.setDescription( i18n( "Select a folder for this trip" ) );
  if ( dlg.exec() ) {
    m_trip->setCollection(m_role, dlg.selectedCollection());
  }
}

Trip* FolderContentsWidget::trip() const
{
  return m_trip;
}



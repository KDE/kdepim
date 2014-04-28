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

// READ THE README FILE

#include "noteviewer.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QEvent>

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/item.h>
#include <AkonadiCore/itemcreatejob.h>
#include <AkonadiCore/itemmodifyjob.h>

#include <KMime/Message>

#include "klocale.h"

using namespace Akonadi;

NoteViewer::NoteViewer(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  m_titleEdit = new QLineEdit(this);
  m_titleEdit->installEventFilter(this);
  m_contentEdit = new QTextEdit(this);
  m_contentEdit->installEventFilter(this);
  layout->addWidget(m_titleEdit);
  layout->addWidget(m_contentEdit);

  Collection noteResource = Collection(6);

//   Item newItem;
//   newItem.setMimeType( "text/x-vnd.akonadi.note" );
//
//   KMime::Message::Ptr newPage = KMime::Message::Ptr( new KMime::Message() );
//
//   QString title = i18nc( "The default name for new pages.", "New Page" );
//   QByteArray encoding( "utf-8" );
//
//   newPage->subject( true )->fromUnicodeString( title, encoding );
//   newPage->contentType( true )->setMimeType( "text/plain" );
//   newPage->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
//   newPage->from( true )->fromUnicodeString( "Kjots@kde4", encoding );
//   // Need a non-empty body part so that the serializer regards this as a valid message.
//   newPage->mainBodyPart()->fromUnicodeString( " " );
//
//   newPage->assemble();
//
//   newItem.setPayload( newPage );
//
//   Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( newItem, noteResource, this );
//   connect( job, SIGNAL(result(KJob*)), SLOT(newPageResult(KJob*)) );

}

void NoteViewer::setIndex(const QPersistentModelIndex& index)
{
  m_persistentIndex = index;
  connect(m_persistentIndex.model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged(QModelIndex,QModelIndex)));
  populateWidget(m_persistentIndex);
}

void NoteViewer::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  QModelIndex idx = topLeft;
  while(idx.isValid())
  {
    if (idx == m_persistentIndex)
    {
      populateWidget(idx);
      return;
    }
    if (idx.column() == bottomRight.column())
      return;
    idx.sibling(idx.row() + 1, idx.column());
  }
}

void NoteViewer::populateWidget(const QModelIndex& index)
{
  Item item = index.data(EntityTreeModel::ItemRole).value<Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;
  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  if (!note)
    return;

  m_titleEdit->setText( note->subject()->asUnicodeString() );

  m_contentEdit->setText( note->mainBodyPart()->decodedText() );
}

bool NoteViewer::eventFilter(QObject* watched, QEvent* event)
{
  if ( ( event->type() == QEvent::FocusOut )
    && ( m_contentEdit->document()->isModified() || m_titleEdit->isModified() )
    && ( watched == m_contentEdit || watched == m_titleEdit ) )
  {
    Item item = m_persistentIndex.data(EntityTreeModel::ItemRole).value<Item>();
    if ( !item.hasPayload<KMime::Message::Ptr>() )
      return false;

    KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

    QByteArray encoding = "utf-8";

    msg->subject()->fromUnicodeString( m_titleEdit->text(), encoding );
    msg->mainBodyPart()->fromUnicodeString( m_contentEdit->toPlainText() );
    msg->assemble();
    item.setPayload( msg );

    ItemModifyJob *modifyJob = new ItemModifyJob(item, this);
    connect(modifyJob, SIGNAL(result(KJob*)), SLOT(modifyDone(KJob*)) );

    m_contentEdit->document()->setModified( false );
    m_titleEdit->setModified( false );
  }
  return QObject::eventFilter(watched,event);
}

void NoteViewer::modifyDone( KJob *job )
{
  if ( job->error() )
  {
    qDebug() << job->errorString();
  }
}


/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "snippetseditor.h"

#include <kactioncollection.h>
#include <kdescendantsproxymodel.h>
#include <mailcommon/snippets/snippetsmanager.h>

#include <QtCore/QAbstractItemModel>
#include <QItemSelectionModel>

SnippetsEditor::SnippetsEditor( KActionCollection *actionCollection, QObject *parent )
  : QObject( parent ), mSnippetsManager( new MailCommon::SnippetsManager( actionCollection, this ) )
{
  actionCollection->addAction(QLatin1String( "snippetseditor_add_snippet"), mSnippetsManager->addSnippetAction() );
  actionCollection->addAction( QLatin1String("snippetseditor_edit_snippet"), mSnippetsManager->editSnippetAction() );
  actionCollection->addAction( QLatin1String("snippetseditor_delete_snippet"), mSnippetsManager->deleteSnippetAction() );

  actionCollection->addAction( QLatin1String("snippetseditor_add_snippetgroup"), mSnippetsManager->addSnippetGroupAction() );
  actionCollection->addAction( QLatin1String("snippetseditor_edit_snippetgroup"), mSnippetsManager->editSnippetGroupAction() );
  actionCollection->addAction( QLatin1String("snippetseditor_delete_snippetgroup"), mSnippetsManager->deleteSnippetGroupAction() );

  actionCollection->addAction( QLatin1String("snippetseditor_insert_snippet"), mSnippetsManager->insertSnippetAction() );

  mProxyModel = new KDescendantsProxyModel( this );
  mProxyModel->setSourceModel( mSnippetsManager->model() );
}

void SnippetsEditor::setEditor( QObject *editor, const char *insertSnippetMethod, const char *dropSignal )
{
  mSnippetsManager->setEditor( editor, insertSnippetMethod, dropSignal );
}

void SnippetsEditor::save()
{
  //TODO:mSnippetsManager->save();
}

QAbstractItemModel* SnippetsEditor::model() const
{
  return mProxyModel;
}

void SnippetsEditor::setRowSelected( int row )
{
  QItemSelectionModel *selectionModel = mSnippetsManager->selectionModel();

  Q_ASSERT( row >= 0 && row < mProxyModel->rowCount() );

  const QModelIndex proxyIndex = mProxyModel->index( row, 0, QModelIndex() );

  selectionModel->select( mProxyModel->mapToSource( proxyIndex ), QItemSelectionModel::ClearAndSelect );
}


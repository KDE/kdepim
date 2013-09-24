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

#include "filtereditor.h"

#include <kactioncollection.h>
#include <mailcommon/filter/filtercontroller.h>

#include <QtCore/QAbstractItemModel>
#include <QItemSelectionModel>

FilterEditor::FilterEditor( KActionCollection *actionCollection, QObject *parent )
  : QObject( parent ), mFilterController( new MailCommon::FilterController( this ) )
{
  actionCollection->addAction( QLatin1String("filtereditor_add"), mFilterController->addAction() );
  actionCollection->addAction( QLatin1String("filtereditor_edit"), mFilterController->editAction() );
  actionCollection->addAction( QLatin1String("filtereditor_delete"), mFilterController->removeAction() );

  actionCollection->addAction( QLatin1String("filtereditor_moveup"), mFilterController->moveUpAction() );
  actionCollection->addAction( QLatin1String("filtereditor_movedown"), mFilterController->moveDownAction() );
}

QAbstractItemModel* FilterEditor::model() const
{
  return mFilterController->model();
}

void FilterEditor::setRowSelected( int row )
{
  QItemSelectionModel *selectionModel = mFilterController->selectionModel();

  selectionModel->select( mFilterController->model()->index( row, 0 ), QItemSelectionModel::ClearAndSelect );
}

#include "filtereditor.moc"

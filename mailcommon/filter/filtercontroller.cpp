/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
    Author: Tobias Koenig <tokoe@kdab.com>

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

#include "filtercontroller.h"
#include "filtereditdialog_p.h"
#include "filtermodel_p.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>

using namespace MailCommon;

class FilterController::Private
{
public:

    void selectionChanged();
    void addFilter();
    void editFilter();
    void removeFilter();
    void moveUpFilter();
    void moveDownFilter();

    FilterModel *mModel;
    QItemSelectionModel *mSelectionModel;
    QAction *mAddAction;
    QAction *mEditAction;
    QAction *mRemoveAction;
    QAction *mMoveUpAction;
    QAction *mMoveDownAction;
};

void FilterController::Private::selectionChanged()
{
    const bool filterSelected = mSelectionModel->hasSelection();

    if ( filterSelected ) {
        mEditAction->setEnabled( true );
        mRemoveAction->setEnabled( true );

        const QModelIndex index = mSelectionModel->selectedRows().first();
        mMoveUpAction->setEnabled( index.row() != 0 );
        mMoveDownAction->setEnabled( index.row() != ( mModel->rowCount() - 1 ) );
    } else {
        mEditAction->setEnabled( false );
        mRemoveAction->setEnabled( false );
        mMoveUpAction->setEnabled( false );
        mMoveDownAction->setEnabled( false );
    }
}

void FilterController::Private::addFilter()
{
    mModel->insertRow( mModel->rowCount() );

    FilterEditDialog dlg;
    dlg.setCaption( i18n( "Add Filter" ) );
    dlg.load( mModel->rowCount() - 1 );

    if ( dlg.exec() ) {
        dlg.save();
    } else {
        mModel->removeRow( mModel->rowCount() - 1 );
    }
}

void FilterController::Private::editFilter()
{
    if ( !mSelectionModel->hasSelection() ) {
        return;
    }

    const QModelIndex index = mSelectionModel->selectedRows().first();

    FilterEditDialog dlg;
    dlg.setCaption( i18n( "Edit Filter" ) );
    dlg.load( index.row() );
    if ( dlg.exec() ) {
        dlg.save();
    }
}

void FilterController::Private::removeFilter()
{
    if ( !mSelectionModel->hasSelection() ) {
        return;
    }

    const QModelIndex index = mSelectionModel->selectedRows().first();

    const int result =
            KMessageBox::questionYesNo(
                0,
                i18n( "Do you really want to remove filter <b>%1</b>?",
                      index.data( Qt::DisplayRole ).toString() ),
                i18n( "Remove Filter" ) );

    if ( result == KMessageBox::No ) {
        return;
    }

    mModel->removeRow( index.row() );
}

void FilterController::Private::moveUpFilter()
{
    if ( !mSelectionModel->hasSelection() ) {
        return;
    }

    const QModelIndex index = mSelectionModel->selectedRows().first();
    mModel->moveRow( index.row(), index.row() - 1 );

    // moveRow will reset the model, so restore the selection
    mSelectionModel->select( mModel->index( index.row() - 1, 0 ),
                             QItemSelectionModel::ClearAndSelect );
}

void FilterController::Private::moveDownFilter()
{
    if ( !mSelectionModel->hasSelection() ) {
        return;
    }

    const QModelIndex index = mSelectionModel->selectedRows().first();
    mModel->moveRow( index.row(), index.row() + 1 );

    // moveRow will reset the model, so restore the selection
    mSelectionModel->select( mModel->index( index.row() + 1, 0 ),
                             QItemSelectionModel::ClearAndSelect );
}

FilterController::FilterController( QObject *parent )
    : QObject( parent ), d( new Private )
{
    d->mModel = new FilterModel( this );
    d->mSelectionModel = new QItemSelectionModel( d->mModel );

    d->mAddAction = new QAction( i18n( "Add" ), this );
    d->mEditAction = new QAction( i18n( "Edit" ), this );
    d->mRemoveAction = new QAction( i18n( "Remove" ), this );
    d->mMoveUpAction = new QAction( i18n( "Move Up" ), this );
    d->mMoveDownAction = new QAction( i18n( "Move Down" ), this );

    connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(selectionChanged()) );

    connect( d->mAddAction, SIGNAL(triggered(bool)), SLOT(addFilter()) );
    connect( d->mEditAction, SIGNAL(triggered(bool)), SLOT(editFilter()) );
    connect( d->mRemoveAction, SIGNAL(triggered(bool)), SLOT(removeFilter()) );
    connect( d->mMoveUpAction, SIGNAL(triggered(bool)), SLOT(moveUpFilter()) );
    connect( d->mMoveDownAction, SIGNAL(triggered(bool)), SLOT(moveDownFilter()) );

    d->selectionChanged();
}

FilterController::~FilterController()
{
    delete d;
}

QAbstractItemModel *FilterController::model() const
{
    return d->mModel;
}

QItemSelectionModel *FilterController::selectionModel() const
{
    return d->mSelectionModel;
}

QAction *FilterController::addAction() const
{
    return d->mAddAction;
}

QAction *FilterController::editAction() const
{
    return d->mEditAction;
}

QAction *FilterController::removeAction() const
{
    return d->mRemoveAction;
}

QAction *FilterController::moveUpAction() const
{
    return d->mMoveUpAction;
}

QAction *FilterController::moveDownAction() const
{
    return d->mMoveDownAction;
}

#include "moc_filtercontroller.cpp"

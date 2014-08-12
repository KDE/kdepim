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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <AkonadiCore/collection.h>

class MainView : public KDeclarativeMainView
{
  Q_OBJECT

  public:
    explicit MainView( QWidget *parent = 0 );

  public slots:
    QString noteTitle( int row ) const;
    QString noteContent( int row ) const;

    void saveNote( const QString &title, const QString &content );
    void saveCurrentNoteTitle( const QString &title );
    void saveCurrentNoteContent( const QString &content );

    void startComposer();

  protected:
    virtual void doDelayedInit();
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createItemFilterModel() const;

    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;

  private:
    Akonadi::Collection suitableContainerCollection( const QModelIndex &parent = QModelIndex() ) const;
};

#endif // MAINVIEW_H

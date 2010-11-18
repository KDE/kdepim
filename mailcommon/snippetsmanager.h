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

#ifndef MAILCOMMON_SNIPPETSMANAGER_H
#define MAILCOMMON_SNIPPETSMANAGER_H

#include "mailcommon_export.h"

#include <QtCore/QObject>

class KActionCollection;

class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

namespace MailCommon {

class MAILCOMMON_EXPORT SnippetsManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QAbstractItemModel* model READ model )
  Q_PROPERTY( QItemSelectionModel* selectionModel READ selectionModel )
  Q_PROPERTY( QAction* addSnippetAction READ addSnippetAction )
  Q_PROPERTY( QAction* editSnippetAction READ editSnippetAction )
  Q_PROPERTY( QAction* deleteSnippetAction READ deleteSnippetAction )
  Q_PROPERTY( QAction* addSnippetGroupAction READ addSnippetGroupAction )
  Q_PROPERTY( QAction* editSnippetGroupAction READ editSnippetGroupAction )
  Q_PROPERTY( QAction* deleteSnippetGroupAction READ deleteSnippetGroupAction )

  public:
    /**
     * Creates a new snippets manager.
     *
     * @param actionCollection The action collection where the manager will register the snippet shortcuts at.
     * @param parent The parent object.
     */
    explicit SnippetsManager( KActionCollection *actionCollection, QObject *parent = 0 );

    /**
     * Destroys the snippets manager.
     */
    ~SnippetsManager();

    /**
     * Returns the model that represents the snippets.
     */
    QAbstractItemModel *model() const;

    /**
     * Returns the selection model that is used by the manager to select the
     * snippet or snippet group to work on.
     */
    QItemSelectionModel *selectionModel() const;

    /**
     * Returns the action that handles adding new snippets.
     */
    QAction *addSnippetAction() const;

    /**
     * Returns the action that handles editing the currently selected snippet.
     */
    QAction *editSnippetAction() const;

    /**
     * Returns the action that handles deleting the currently selected snippet.
     */
    QAction *deleteSnippetAction() const;

    /**
     * Returns the action that handles adding new snippet groups.
     */
    QAction *addSnippetGroupAction() const;

    /**
     * Returns the action that handles editing the currently selected snippet group.
     */
    QAction *editSnippetGroupAction() const;

    /**
     * Returns the action that handles deleting the currently selected snippet group.
     */
    QAction *deleteSnippetGroupAction() const;

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void selectionChanged() )
    Q_PRIVATE_SLOT( d, void addSnippet() )
    Q_PRIVATE_SLOT( d, void editSnippet() )
    Q_PRIVATE_SLOT( d, void deleteSnippet() )
    Q_PRIVATE_SLOT( d, void addSnippetGroup() )
    Q_PRIVATE_SLOT( d, void editSnippetGroup() )
    Q_PRIVATE_SLOT( d, void deleteSnippetGroup() )
    //@endcond
};

}

#endif

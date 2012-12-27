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

#ifndef SNIPPETSEDITOR_H
#define SNIPPETSEDITOR_H

#include <QtCore/QObject>

namespace MailCommon {
class SnippetsManager;
}

class KActionCollection;
class KDescendantsProxyModel;
class QAbstractItemModel;

/**
 * @short The C++ part of the snippets editor for mobile apps.
 *
 * This class encapsulates the logic of the snippet viewing/editing
 * and the UI is provided by SnippetsEditor.qml.
 */
class SnippetsEditor : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new snippets editor.
     *
     * @param actionCollection The action collection to register the manipulation
     *                         actions (e.g. add, edit, delete) at
     * @param parent The parent object.
     */
    explicit SnippetsEditor( KActionCollection *actionCollection, QObject *parent = 0 );

    void setEditor( QObject *editor, const char *insertSnippetMethod, const char *dropSignal );

  public Q_SLOTS:
    /**
     * Saves changes to the snippets back to disk.
     */
    void save();

    /**
     * Returns the snippets list model of the current collection.
     */
    QAbstractItemModel* model() const;

    /**
     * Sets the row of the snippets the user has selected in the UI.
     */
    void setRowSelected( int row );

  private:
    MailCommon::SnippetsManager *mSnippetsManager;
    KDescendantsProxyModel *mProxyModel;
};

#endif

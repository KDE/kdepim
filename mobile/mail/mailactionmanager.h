/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#ifndef MAILACTIONMANAGER_H
#define MAILACTIONMANAGER_H

#include <QItemSelectionModel>

#include "kactioncollection.h"

class MailActionManager : public QObject
{
  Q_OBJECT

  public:
    explicit MailActionManager( KActionCollection *actionCollection, QObject *parent = 0 );

    void setItemSelectionModel( QItemSelectionModel *selectionModel );
    void setItemActionSelectionModel( QItemSelectionModel *selectionModel );

  private slots:
    void updateActions();

  private:
    KActionCollection *m_actionCollection;
    QItemSelectionModel *m_itemSelectionModel;
    QItemSelectionModel *m_itemActionSelectionModel;
};

#endif // MAILACTIONMANAGER_H


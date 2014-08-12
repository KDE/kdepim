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

#ifndef MESSAGELISTSETTINGSCONTROLLER_H
#define MESSAGELISTSETTINGSCONTROLLER_H

#include "messagelistsettings.h"

#include <AkonadiCore/collection.h>

#include <QtCore/QObject>

class QAction;

class MessageListSettingsController : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QString groupingRole READ groupingRole NOTIFY settingsChanged )
  Q_PROPERTY( QAction* editAction READ editAction )

  public:
    explicit MessageListSettingsController( QObject *parent = 0 );

    QString groupingRole() const;

    QAction* editAction() const;

  public Q_SLOTS:
    void setCollection( const Akonadi::Collection &collection );

  Q_SIGNALS:
    void settingsChanged( const MessageListSettings &settings );

  private Q_SLOTS:
    void editSettings();

  private:
    Akonadi::Collection::Id mCollectionId;
    MessageListSettings mSettings;
    QAction *mEditAction;
};

#endif

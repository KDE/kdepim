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

#ifndef MESSAGEVIEWER_MESSAGEREADHANDLER_H
#define MESSAGEVIEWER_MESSAGEREADHANDLER_H

#include "messageviewer_export.h"

#include "viewer/viewer.h"

#include <QtCore/QObject>

namespace MessageViewer {

/**
 * @short A viewer handler to mark viewed messages as read.
 */
class MESSAGEVIEWER_EXPORT MarkMessageReadHandler : public QObject, public AbstractMessageLoadedHandler
{
    Q_OBJECT

public:
    /**
     * Creates a new mark message read handler.
     *
     * @param parent The parent object.
     */
    explicit MarkMessageReadHandler( QObject *parent = 0 );

    /**
     * Destroys the mark message read handler.
     */
    ~MarkMessageReadHandler();

    /**
     * @copydoc AbstractMessageLoadedHandler::setItem()
     */
    void setItem( const Akonadi::Item &item );

private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void handleMessages() )
    //@endcond
};

}

#endif

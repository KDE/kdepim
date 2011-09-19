/*
    This file is part of Akregator.

    Copyright (C) 2011 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_MODIFYCOMMANDS_H
#define AKREGATOR_MODIFYCOMMANDS_H

#include "command.h"

namespace Akonadi {
   class Collection;
   class Session;
}

namespace Akregator {
class MarkAsReadCommand : public Command {
    Q_OBJECT
public:
    explicit MarkAsReadCommand( QObject* parent=0 );

    void setCollection( const Akonadi::Collection& c );
    void setSession( Akonadi::Session* s );

private:
    void doStart();

private Q_SLOTS:
    void collectionsFetched( KJob* );
    void itemsFetched( KJob* );
    void itemsModified( KJob* );

private:
    class Private;
    Private* const d;
};

}

#endif


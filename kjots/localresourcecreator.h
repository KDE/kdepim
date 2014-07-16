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

#ifndef LOCALRESOURCECREATOR_H
#define LOCALRESOURCECREATOR_H

#include <QObject>

#include "noteshared/resources/localresourcecreator.h"

class KJob;

/**
 * @brief Creates a notes resource, a book and a page if one does not already exist.
 */
class LocalResourceCreator : public NoteShared::LocalResourceCreator
{
  Q_OBJECT
public:
  explicit LocalResourceCreator(QObject* parent = 0);


protected:
    virtual void finishCreateResource();

private slots:
  void rootFetchFinished( KJob *job );
  void topLevelFetchFinished( KJob *job );
  void createFinished( KJob *job );
  void itemCreateFinished( KJob *job );

};

#endif

/*
    This file is part of kdepim.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <qobject.h>

/**
  A small helper class which blocks an asynchronous operation (e.g. a KIO request)
  so that it can be used in a synchronous environment.

  Example:

    ...

    Synchronizer mSynchronizer;

    ...

    job = KIO::file_copy( url, file, -1, true );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotResult( KIO::Job * ) ) );

    mSynchronizer.start(); // will block here until the slot was called
    ...


    void slotResult( KIO::Job* )
    {
      mSynchronizer.stop();
    }
 */
class Synchronizer
{
  public:
    Synchronizer();

    /**
      Blocks the execution until @ref stop() is called.
     */
    void start();

    /**
      Unblocks the execution.
     */
    void stop();

  private:
    bool mBlocked;
};

#endif


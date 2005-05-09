/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef UIDMAPPER_H
#define UIDMAPPER_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>
#include <qvariant.h>

/**
  A helper class which maps between to kinds of uids and can load/store
  these information to a file.
*/
class UIDMapper : public QObject
{
  Q_OBJECT

  public:

    /**
      The constructor.

      @param fileName The file where the map shall be saved / loaded from.
     */
    UIDMapper( const QString fileName );

    virtual ~UIDMapper();

    /**
      Loads the map from the file specified in the constructor.
     */
    void load();

    /**
      Saves the map to the file specified in the constructor.
     */
    void store();

    /**
      Adds an uid pair.

      @param local The local uid.
      @param remote The remote uid.
     */
    void add( const QString& local, const QString &remote );

    /**
      Removes the pair with the given local uid.

      @param local The local uid.
     */
    void removeByLocal( const QString& local );

    /**
      Removes the pair with the given remote uid.

      @param local The local uid.
     */
    void removeByRemote( const QString& remote );

    /**
      Returns the remote uid which belongs to the given local uid.

      @param local The local uid.
     */
    QString remoteUid( const QString& local ) const;

    /**
      Returns the local uid which belongs to the given remote uid.

      @param local The local uid.
     */
    QString localUid( const QString& remote ) const;

  private:
    QMap<QString, QVariant> mMap;

    QString mFileName;
};

#endif

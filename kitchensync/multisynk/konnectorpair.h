/*
    This file is part of KitchenSync.

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

#ifndef KONNECTORPAIR_H
#define KONNECTORPAIR_H

#include <qmap.h>
#include <qvaluelist.h>

#include "konnectormanager.h"

class KonnectorPair
{
  public:
    typedef QValueList<KonnectorPair*> List;
    typedef QMap<QString, KonnectorPair*> Map;

    enum ResolveStrategy
    {
      ResolveManually,
      ResolveFirst,
      ResolveSecond,
      ResolveBoth
    };

    KonnectorPair();
    ~KonnectorPair();

    QString uid() const;
    void setUid( const QString &uid );

    QString name() const;
    void setName( const QString &name );

    int resolveStrategy() const;
    void setResolveStrategy( int strategy );

    void load();
    void save();

    KonnectorManager* manager();

  private:
    QString configFile() const;

    QString mUid;
    QString mName;
    int mStrategy;

    KonnectorManager *mManager;
    KConfig *mConfig;
};

#endif

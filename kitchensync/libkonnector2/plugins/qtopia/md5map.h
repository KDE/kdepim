/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
#ifndef MD5_MAP_H
#define MD5_MAP_H

#include <qcstring.h>
#include <qmap.h>

/**
 * MD5 Map is here to keep
 * a Map of UID and a MD5 SUM
 * together
 * It can save/load, find and
 * iterate over a list of items
 */
class KConfig;

namespace OpieHelper {
    class MD5Map {
    public:
        typedef QMap<QString, QString> Map;
        typedef QMap<QString, QString>::Iterator Iterator;
        MD5Map(const QString& fileName = QString::null );
        ~MD5Map();
        void load( const QString& fileName );

        /* clears before saving */
        void save();
        /* only works if not loaded before */
        void setFileName( const QString& );

        QString md5sum(const QString& )const;
        bool contains( const QString& )const;
        void insert( const QString& , const QString& );
        void set( const Map& map );

        Map map()const;

        void clear();

    protected:
        KConfig* config();

    private:
        KConfig* m_conf;
        Map m_map;
        QString m_file;
    };
};

#endif

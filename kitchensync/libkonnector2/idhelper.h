/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

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
#ifndef idhelper_h
#define idhelper_h

#include <qmap.h>
#include <qvaluelist.h>

#include <kontainer.h> // from libksyncentry

class KConfig;
/**
 * the KonnectorUIDHelper helps to manage the
 * relation between the KDE uids and the KonnectorPlugin
 * uids. This makes finding ids later more easy
 */
namespace KSync {
    class KDE_EXPORT KonnectorUIDHelper {
    public:
        // the full path to the dir where the file is stored
        /**
         * c'tor
         * @param dir The directory name where the relations
         *            between KDE and the Konnector UIDs
         *            are saved
         */
        KonnectorUIDHelper( const QString &dir );
        ~KonnectorUIDHelper();

        /**
         * @param appName The Application Name. For example 'datebook'
         * @param kdeId the UID assigned to in the KDE world
         * @param defaultId in case of failure what should be returned
         *
         * @return returns a to the Konnector known uid
         */
        QString konnectorId( const QString &appName,
                             const QString &kdeId,
                             const QString &defaultId = QString::null );

        /**
         * fetches a KDE UID for an appName
         * and a konnectorUID
         */
        QString kdeId( const QString &appName,
                       const QString &konnectorId,
                       const QString &defaultId = QString::null );

        void addId(const QString& appName,
                   const QString& konnectorId,
                   const QString& kdeId);
        void replaceIds( const QString& appName,
                         Kontainer::ValueList );
        void removeId(const QString &app,  const QString &id);
        void clear();
        void save();

    private:
        class KonnectorUIDHelperPrivate;
        KonnectorUIDHelperPrivate *d;
        KConfig *m_config;
        QMap< QString,  Kontainer::ValueList > m_ids;
    };
}
#endif

/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KITCHENSYNC_PROFILE_H
#define KITCHENSYNC_PROFILE_H

#include <qmap.h>
#include <qstring.h>

#include "actionpartservice.h"

#include <kdepimmacros.h>

namespace KSync {

/**
 * A Profile keeps user settings like a name,
 * the list of plugins to be loaded on activation,
 * a list of where to read data from....
 */
class KDE_EXPORT Profile
{
  public:
    typedef QMap<QString,  QString> PathMap;
    bool operator==( const Profile& );
//        bool operator!=( const Profile& a) { return !(a == *this); };
    typedef QValueList<Profile> List;

    /**
     * constructs an empty Profile
     * and generates a uid
     */
    Profile();

    /**
     * copy c'tor
     */
    Profile( const Profile & );

    /**
     * destructs a Profile
     */
    ~Profile();

    /**
     * @return the user given name of the profile
     */
    QString name() const;

    /**
     * @return the uid of the Profile
     */
    QString uid() const;

    /**
     * @return a name of a Pixmap the use
     * chose to associate
     */
    QString pixmap() const;

    /**
     * @return if the write back should be confirmed by
     * the user
     */
    bool confirmSync() const;

    /**
     * @return if the deletion of Entries should be confirmed
     * by the user
     */
    bool confirmDelete() const;

    /**
     * set the name
     * @param name the name of the Profile
     */
    void setName( const QString &name ) ;

    /**
     * set the uid
     * @param id the id of the Profile
     */
    void setUid( const QString &id );

    /**
     * set the Pixmap name
     * @param pix The pixmap
     */
    void setPixmap( const QString &pix);

    /**
     * @return the ActionParts to be loaded for
     * the profile
     */
    ActionPartService::List actionParts() const;

    /**
     * set which parts to be loaded
     * @param lst The list of ActionPartServices
     */
    void setActionParts( const ActionPartService::List &lst );

    /**
     * Parts can save the file location inside a Profile
     * path returns the PATH for a part
     */
    QString path( const QString &partName ) const;

    /**
     * sets the path for a partName
     * to path
     * @param partName The part name
     * @param path the path
     */
    void setPath( const QString &partName, const QString &path );

    /**
     * sets the path map
     */
    void setPaths( const PathMap & );

    /**
     * returns the PathMap
     */
    PathMap paths() const;

    /**
     * set if the user wants to confirm sync
     */
    void setConfirmSync( bool );

    /**
     * set if the user wants to confirm deletions
     */
    void setConfirmDelete( bool );

    /**
     * copy operator;
     */
    Profile &operator=( const Profile & );

  private:
    QString m_name;
    QString m_uid;
    QString m_pixmap;
    ActionPartService::List m_actionPartServices;
    PathMap m_map;
    bool m_confirmSync   : 1;
    bool m_confirmDelete : 1;
};

}

#endif

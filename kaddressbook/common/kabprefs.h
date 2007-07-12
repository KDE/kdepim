/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KABPREFS_H
#define KABPREFS_H

#include "kabcommon_export.h"
#include "kabprefs_base.h"
#include <QStringList>

class KABCOMMON_EXPORT KABPrefs : public KABPrefsBase
{
  public:
    virtual ~KABPrefs();

    static KABPrefs *instance();

    void usrReadConfig();
    void usrWriteConfig();

    void setLocationMapURL( const QString &locationMapURL )
    {
      if ( !isImmutable( QString::fromLatin1( "LocationMapURL" ) ) )
        mLocationMapURL = locationMapURL;
    }

    QString locationMapURL() const
    {
      return mLocationMapURL;
    }

    void setLocationMapURLs( const QStringList &locationMapURLs )
    {
      if ( !isImmutable( QString::fromLatin1( "LocationMapURLs" ) ) )
        mLocationMapURLs = locationMapURLs;
    }

    QStringList locationMapURLs() const
    {
      return mLocationMapURLs;
    }

    QStringList customCategories() const
    {
      return mCustomCategories;
    }

    void setCustomCategories(const QStringList & s)
    {
      mCustomCategories = s;
    }

    void setCategoryDefaults();

  private:
    KABPrefs();

    static KABPrefs *mInstance;

    QString mLocationMapURL;
    QStringList mLocationMapURLs;
};

#endif

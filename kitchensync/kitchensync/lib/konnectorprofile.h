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
#ifndef KSYNC_KONNECTOR_PROFILE_H
#define KSYNC_KONNECTOR_PROFILE_H

#include <qstring.h>
#include <qvaluelist.h>

#include <kapabilities.h>
#include <kdevice.h>

class KConfig;

namespace KSync {

/**
 * A KonnectorProfile stores the name and a KSync::Device,
 * a Kapabilities and other configuration related
 * to a Konnector
 * @base a simple KonnectorProfile
 * @author Holger 'zecke' Freyther
 */
class KonnectorProfile
{
  public:
    typedef QValueList<KonnectorProfile> ValueList;

    /**
     * a simple c'tor
     * @base simple c'tor
     */
    KonnectorProfile();

    /**
     * Another constructor
     * @param name The name of the Konnector assigned by the user
     * @param icon A user supplied icon
     * @param Device the device
     */
    KonnectorProfile( const QString &name,
                      const QString &icon,
                      const Device &dev );

    /**
     * A simple copy constructor
     */
    KonnectorProfile( const KonnectorProfile & );
    ~KonnectorProfile();

    bool operator==( const KonnectorProfile & );
    bool operator==( const KonnectorProfile & ) const;
    KonnectorProfile &operator=( const KonnectorProfile & );

    /**
     * The UID gets assigned by the MainWindow
     * @see MainWindow
     * @return the UID of the Konnector
     */
    QString uid() const;

    /**
     * @return the user supplied name
     */
    QString name() const;

    /**
     * @return the icon name
     */
    QString icon() const;

    /**
     * @return the device
     */
    Device device() const;

    /**
      @return the Konnector object asscociated with the profile. Can be 0, if
              profile isn't loaded.
    */
    Konnector *konnector() const;

    /**
     * @return the kapabilities  of this KonnectorProfile
     * @see Kapabilities
     */
    Kapabilities kapabilities() const;

    /**
     * @return if the KonnectorProfile was
     * 	   loaded in a previous session
     */
    bool wasLoaded() const;

    /**
     * set the uid of the KonnectorProfile
     * @param uid The uid of the KonnectorProfile
     */
    void setUid( const QString& uid );

    /**
     * set the name of the KonnectorProfile
     * @param name The name of the KonnectorProfile
     */
    void setName( const QString& name );

    /**
     * set the icon of the KonnectorProfile
     * @param icon the icon name
     */
    void setIcon( const QString& icon );

    /**
     * set the Device of this KonnectorProfile
     * @param dev The device
     */
    void setDevice( const Device& dev );

    /**
      Set the Konnector the profile belongs to
      
      @param konnector pointer to Konnector object
    */
    void setKonnector( Konnector * );

    /**
     * set the Kapabilities for the KonnectorProfile
     * @param caps The Kapabilities
     */
    void setKapabilities( const Kapabilities& caps );

    /**
     * @return if this KonnectorProfile is valid
     * It is valid if it got a proper name!
     */
    bool isValid() const;

    /**
     * @internal used for storing to a KConfig
     */
    void saveToConfig( KConfig * ) const;

    /**
     * @internal loading from a Kconfig
     */
    void loadFromConfig( KConfig * );

  private:
    void saveKaps( KConfig * ) const;
    Kapabilities readKaps( KConfig * );

    bool m_wasLoaded :1;
    QString m_name;
    QString m_icon;
    Device m_dev;
    QString m_uid;
    Konnector *m_konnector;
    Kapabilities m_caps;

    class KonnectorProfilePrivate;
    KonnectorProfilePrivate *d;
};

}

#endif

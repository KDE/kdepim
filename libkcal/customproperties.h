/*
    This file is part of libkcal.

    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCAL_CUSTOMPROPERTIES_H
#define KCAL_CUSTOMPROPERTIES_H

#include <QString>
#include <QMap>
//Added by qt3to4:
#include <QByteArray>
#include <kdepimmacros.h>

#include "libkcal_export.h"

namespace KCal {

/**
  This class represents custom calendar properties.
  It is used as a base class for classes which represent calendar components.
  A custom property name written by libkcal has the form X-KDE-APP-KEY where
  APP represents the application name, and KEY distinguishes individual
  properties for the application.
  In keeping with RFC2445, property names must be composed only of the
  characters A-Z, a-z, 0-9 and '-'.
*/
class LIBKCAL_EXPORT CustomProperties
{
  public:
    /**
      Construct a new empty custom properties instance.
    */
    CustomProperties();
    CustomProperties( const CustomProperties & );
    ~CustomProperties();

    bool operator==( const CustomProperties & ) const;

    /**
      Create or modify a custom calendar property.
      
      @param app   Application name as it appears in the custom property name.
      @param key   Property identifier specific to the application.
      @param value The property's value. A call with a value of QString()
                   will be ignored.
    */
    void setCustomProperty( const QByteArray &app, const QByteArray &key,
                            const QString &value );
    /**
      Delete a custom calendar property.
      
      @param app Application name as it appears in the custom property name.
      @param key Property identifier specific to the application.
    */
    void removeCustomProperty( const QByteArray &app, const QByteArray &key );
    /**
      Return the value of a custom calendar property.
      
      @param app Application name as it appears in the custom property name.
      @param key Property identifier specific to the application.
      @return Property value, or QString() if (and only if) the property
              does not exist.
    */
    QString customProperty( const QByteArray &app, const QByteArray &key ) const;

    /**
      Create or modify a non-KDE or non-standard custom calendar property.
      
      @param name Full property name
      @param value The property's value. A call with a value of QString()
                   will be ignored.
    */
    void setNonKDECustomProperty( const QByteArray &name, const QString &value );
    /**
      Delete a non-KDE or non-standard custom calendar property.
      
      @param name Full property name
    */
    void removeNonKDECustomProperty( const QByteArray &name );
    /**
      Return the value of a non-KDE or non-standard custom calendar property.
      
      @param name Full property name
      @return Property value, or QString() if (and only if) the property
              does not exist.
    */
    QString nonKDECustomProperty( const QByteArray& name ) const;

    /**
      Initialise the alarm's custom calendar properties to the specified
      key/value pairs.
    */
    void setCustomProperties( const QMap<QByteArray, QString> &properties );
    /**
      Return all custom calendar property key/value pairs.
    */
    QMap<QByteArray, QString> customProperties() const;

  private:
    static bool checkName(const QByteArray& name);

    QMap<QByteArray, QString> mProperties;   // custom calendar properties

    class Private;
    Private *d;
};

}

#endif

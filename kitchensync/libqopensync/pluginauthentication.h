/*
    This file is part of libqopensync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#ifndef QSYNC_PLUGINAUTHENTICATION_H
#define QSYNC_PLUGINAUTHENTICATION_H

#include <libqopensync/qopensync_export.h>

#include <QtCore/QString>

class OSyncPluginAuthentication;

namespace QSync {

class QSYNC_EXPORT PluginAuthentication
{
  public:
    PluginAuthentication();
    ~PluginAuthentication();

    /**
      Returns whether the object is a valid plugin authentication.
     */
    bool isValid() const;

    /**
      Sets the user name.
     */
    void setUserName( const QString &userName );

    /**
      Returns the user name.
     */
    QString userName() const;

    /**
      Sets the password.
     */
    void setPassword( const QString &password );

    /**
      Returns the password.
     */
    QString password() const;

    /**
      Sets the reference.
     */
    void setReference( const QString &reference );

    /**
      Returns the reference.
     */
    QString reference() const;

  private:
    OSyncPluginAuthentication *mPluginAuthentication;
};

}

#endif

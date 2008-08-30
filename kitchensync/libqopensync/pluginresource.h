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

#ifndef QSYNC_PLUGINRESOURCE_H
#define QSYNC_PLUGINRESOURCE_H

#include <libqopensync/qopensync_export.h>
#include <libqopensync/objectformatsink.h>

#include <QtCore/QList>
#include <QtCore/QString>

class OSyncPluginResource;

namespace QSync {

class QSYNC_EXPORT PluginResource
{
    friend class PluginConfig;

  public:
    typedef QList<PluginResource> List;

    PluginResource();
    ~PluginResource();

    /**
      Returns whether the object is a valid plugin resource.
     */
    bool isValid() const;

    /**
      Sets whether the resource is enabled.
     */
    void setEnabled( bool enabled );

    /**
      Returns whether the resource is enabled.
     */
    bool enabled() const;

    /**
      Sets the name of the resource.
     */
    void setName( const QString &name );

    /**
      Returns the name of the resource.
     */
    QString name() const;

    /**
      Sets the mime type of the resource.
     */
    void setMimeType( const QString &mimeType );

    /**
      Returns the mime type of the resource.
     */
    QString mimeType() const;

    /**
      Returns the list of available object format sinks
     */
    ObjectFormatSink::List objectFormatSinks() const;

    /**
      Adds an object format sink.
     */
    void addObjectFormatSink( const ObjectFormatSink &sink );

    /**
      Remove object format sink.
     */
    void removeObjectFormatSink( const ObjectFormatSink &sink );

    /**
      Sets the object type of the resource.
     */
    void setObjectType( const QString &objectType );

    /**
      Returns the object type of the resource.
     */
    QString objectType() const;

    /**
      Sets the path of the resource.
     */
    void setPath( const QString &path );

    /**
      Returns the path of the resource.
     */
    QString path() const;

    /**
      Sets the url of the resource.
     */
    void setUrl( const QString &url );

    /**
      Returns the url of the resource.
     */
    QString url() const;

  private:
    OSyncPluginResource *mPluginResource;
};

}

#endif

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

#ifndef QSYNC_OBJECTFORMATSINK_H
#define QSYNC_OBJECTFORMATSINK_H

#include <QtCore/QList>
#include <QtCore/QString>

#include <libqopensync/qopensync_export.h>

class OSyncObjFormatSink;

namespace QSync {

class QSYNC_EXPORT ObjectFormatSink
{
    friend class PluginResource;

  public:
    typedef QList<ObjectFormatSink> List;

    ObjectFormatSink();
    ~ObjectFormatSink();

    /**
      Returns whether the object format sink is a valid group.
     */
    bool isValid() const;

    /**
      Returns the object format of the format sink.
     */
    QString objectFormat() const;

    /**
      Sets the object configuration of the format sink.
     */
    void setConfiguration( const QString &config );

    /**
      Returns the object configuration of the format sink.
     */
    QString configuration() const;

  private:
    OSyncObjFormatSink *mObjectFormatSink;
};

}

#endif

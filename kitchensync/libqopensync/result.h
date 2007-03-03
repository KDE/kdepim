/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef QSYNC_RESULT_H
#define QSYNC_RESULT_H

#include <qstring.h>

struct OSyncError;

namespace QSync {

class Result
{
  public:
    /**
      Result types.
     */
    enum Type {
      NoError,
      GenericError,
      IOError,
      NotSupported,
      Timeout,
      Disconnected,
      FileNotFound,
      Exists,
      Convert,
      Misconfiguration,
      Initialization,
      Parameter,
      Expected,
      NoConnection,
      Temporary,
      Locked,
      PluginNotFound
    };

    /**
      Constructs a NoError result.
     */
    Result();

    /**
      Constructs a result of the given type.
     */
    Result( Type type );

    /**
      Construct Result from OpenSync error object. Deletes the OpenSync error
      object.
    */
    Result( OSyncError **, bool deleteError = true );

    /**
      Destroys the result.
     */
    ~Result();

    /**
      Sets the name of the result.
     */
    void setName( const QString &name );

    /**
      Returns the name of the result.
     */
    QString name() const;

    /**
      Sets the message text of the result.
     */
    void setMessage( const QString &message );

    /**
      Returns the message text of the result.
     */
    QString message() const;

    /**
      Sets the type of the result.
     */
    void setType( Type type );

    /**
      Returns the type of the result.
     */
    Type type() const;

    /**
      Reimplemented boolean operator.
     */
    operator bool () const;

    /**
      Return true, if this Result is an error, return false otherwise.
    */
    bool isError() const;

  private:
    QString mName;
    QString mMessage;
    Type mType;
};

}

#endif

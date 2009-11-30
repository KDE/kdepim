/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef TASK_H
#define TASK_H

#include <QtCore/QObject>

namespace GpgME {
  class Context;
}

/** Shared base class for tasks performed by the Nepomuk Email agent. */
class Task : public QObject
{
  Q_OBJECT
  public:
    Task( QObject *parent = 0 );
    ~Task();

    bool createCryptoContainer( const QByteArray& keyId );
    bool mountCryptoContainer( const QByteArray& keyId );
    void unmountCryptoContainer();
    QString containerPathFromKeyId( const QByteArray& keyId );
    QString repositoryPathFromKeyId( const QByteArray &keyId );

  private:
    GpgME::Context* m_ctx;
};

#endif // TASK_H

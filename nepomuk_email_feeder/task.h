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

namespace Soprano {
  class Model;
}

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
    static QList<QByteArray> listCryptoContainers();
    static QString containerPathFromKeyId( const QByteArray& keyId );
    static QString repositoryPathFromKeyId( const QByteArray &keyId );

    Soprano::Model* cryptoModel( const QByteArray &keyId );
    inline bool hasActiveCryptoModel() const { return m_cryptoModel; }
    void resetModel();

  private:
    GpgME::Context* m_ctx;
    Soprano::Model* m_cryptoModel;
};

#endif // TASK_H

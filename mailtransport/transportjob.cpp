/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "transport.h"
#include "transportjob.h"

#include <qbuffer.h>

using namespace KPIM;

class KPIM::TransportJob::Private
{
  public:
    Transport* transport;
    QString sender;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    QByteArray data;
    QBuffer *buffer;
};

TransportJob::TransportJob( Transport* transport, QObject * parent ) :
    KCompositeJob( parent ),
    d( new Private )
{
  d->transport = transport;
  d->buffer = 0;
}

TransportJob::~ TransportJob()
{
  delete d;
}

void TransportJob::setSender(const QString & sender)
{
  d->sender = sender;
}

void TransportJob::setTo(const QStringList &to)
{
  d->to = to;
}

void TransportJob::setCc(const QStringList &cc)
{
  d->cc = cc;
}

void TransportJob::setBcc(const QStringList &bcc)
{
  d->bcc = bcc;
}

void TransportJob::setData(const QByteArray & data)
{
  d->data = data;
}

Transport* TransportJob::transport() const
{
  return d->transport;
}

QString TransportJob::sender() const
{
  return d->sender;
}

QStringList TransportJob::to() const
{
  return d->to;
}

QStringList TransportJob::cc() const
{
  return d->cc;
}

QStringList TransportJob::bcc() const
{
  return d->bcc;
}

QByteArray TransportJob::data() const
{
  return d->data;
}

QBuffer* TransportJob::buffer()
{
  if ( !d->buffer ) {
    d->buffer = new QBuffer( this );
    d->buffer->setData( d->data );
    Q_ASSERT( d->buffer->open( QIODevice::ReadOnly ) );
  }
  return d->buffer;
}

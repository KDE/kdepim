/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "infopart.h"

using namespace MessageComposer;

class InfoPart::Private
{
  public:
    QString from;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    QString subject;
    int transportId;
};

InfoPart::InfoPart( QObject *parent )
  : MessagePart( parent )
  , d( new Private )
{
  d->transportId = 0;
}

InfoPart::~InfoPart()
{
  delete d;
}

QString InfoPart::from() const
{
  return d->from;
}

void InfoPart::setFrom( const QString &from )
{
  d->from = from;
}

QStringList InfoPart::to() const
{
  return d->to;
}

void InfoPart::setTo( const QStringList &to )
{
  d->to = to;
}

QStringList InfoPart::cc() const
{
  return d->cc;
}

void InfoPart::setCc( const QStringList &cc )
{
  d->cc = cc;
}

QStringList InfoPart::bcc() const
{
  return d->bcc;
}

void InfoPart::setBcc( const QStringList &bcc )
{
  d->bcc = bcc;
}

QString InfoPart::subject() const
{
  return d->subject;
}

void InfoPart::setSubject( const QString &subject )
{
  d->subject = subject;
}

int InfoPart::transportId() const
{
  return d->transportId;
}

void InfoPart::setTransportId( int tid )
{
  d->transportId = tid;
}

#include "infopart.moc"

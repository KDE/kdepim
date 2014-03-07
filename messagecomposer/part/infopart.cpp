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
    QString fcc;
    QString replyTo;
    QString userAgent;
    QString inReplyTo;
    QString references;
    int transportId;
    bool urgent;
    KMime::Headers::Base::List extraHeaders;
};

InfoPart::InfoPart( QObject *parent )
    : MessagePart( parent )
    , d( new Private )
{
    d->transportId = 0;
    d->urgent = false;
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

QString InfoPart::replyTo() const
{
    return d->replyTo;
}

void InfoPart::setReplyTo(const QString& replyTo)
{
    d->replyTo = replyTo;
}


int InfoPart::transportId() const
{
    return d->transportId;
}

void InfoPart::setTransportId( int tid )
{
    d->transportId = tid;
}

void InfoPart::setFcc( const QString &fcc )
{
    d->fcc = fcc;
}

QString InfoPart::fcc() const
{
    return d->fcc;
}

bool InfoPart::urgent() const
{
    return d->urgent;
}

void InfoPart::setUrgent( bool urgent )
{
    d->urgent = urgent;
}

QString InfoPart::inReplyTo() const
{
    return d->inReplyTo;
}

void InfoPart::setInReplyTo( const QString& inReplyTo )
{
    d->inReplyTo = inReplyTo;
}

QString InfoPart::references() const
{
    return d->references;
}

void InfoPart::setReferences( const QString& references )
{
    d->references = references;
}

void InfoPart::setExtraHeaders( KMime::Headers::Base::List headers )
{
    d->extraHeaders = headers;
}

KMime::Headers::Base::List InfoPart::extraHeaders() const
{
    return d->extraHeaders;
}

QString InfoPart::userAgent() const
{
    return d->userAgent;
}

void InfoPart::setUserAgent ( const QString& userAgent )
{
    d->userAgent = userAgent;
}


/*
Copyright 2014  Abhijeet Nikam connect08nikam@gmail.com

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

#include <composer.h>
#include <KMime/Message>


QString Composer::from() const
{
    return m_from;
}

QStringList Composer::cc() const
{
    return m_cc;
}


QStringList Composer::bcc() const
{
    return m_bcc;
}


QStringList Composer::to() const
{
    return m_to;
}

QString Composer::subject() const
{
    return m_subject;
}

QString Composer::body() const
{
    return m_body;
}


void Composer::setFrom(const QString &from)
{
    if ( from != m_from ) {
        m_from = from;
        emit fromChanged();
    }

}


void Composer::setTo(const QStringList &to)
{
    if ( to != m_to ) {
        m_to = to;
        emit toChanged();
    }
}


void Composer::setCC(const QStringList &cc)
{
    if ( cc != m_cc ) {
        m_cc = cc;
        emit ccChanged();
    }
}


void Composer::setBCC(const QStringList &bcc)
{
    if ( bcc != m_bcc ) {
        m_bcc = bcc;
        emit bccChanged();
    }
}

void Composer::setSubject(const QString &subject)
{
    if ( subject != m_subject ) {
        m_subject = subject;
        emit subjectChanged();
    }
}

void Composer::setBody(const QString &body)
{
    if ( body != m_body ) {
        m_body = body;
        emit bodyChanged();
    }
}

void Composer::send()
{

}

void Composer::sendLater()
{

}

void Composer::saveDraft()
{

}


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

#ifndef MESSAGESEARCH_H
#define MESSAGESEARCH_H

#include <task.h>
#include <akonadi/collection.h>

class KJob;

class MessageSearch : public Task
{
  Q_OBJECT
  public:
    MessageSearch( const QString &query, const Akonadi::Collection &destination, QObject *parent = 0 );

  private:
    void searchInContainer( const QByteArray &keyId );

    void ref();
    void deref();

  private slots:
    void linkResult( KJob* job );

  private:
    QString m_query;
    Akonadi::Collection m_destination;
    int m_refCount;
};

#endif // MESSAGESEARCH_H

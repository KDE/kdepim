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

#ifndef MESSAGECOMPOSER_INFOPART_H
#define MESSAGECOMPOSER_INFOPART_H

#include "messagepart.h"

#include <QtCore/QStringList>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

namespace MessageComposer {

/** setOverrideTransferEncoding for an InfoPart has no effect */
class MESSAGECOMPOSER_EXPORT InfoPart : public MessagePart
{
  Q_OBJECT

  public:
    explicit InfoPart( QObject *parent = 0 );
    virtual ~InfoPart();

    QString from() const;
    void setFrom( const QString &from );
    QStringList to() const;
    void setTo( const QStringList &to );
    QStringList cc() const;
    void setCc( const QStringList &cc );
    QStringList bcc() const;
    void setBcc( const QStringList &bcc );

    QString subject() const;
    void setSubject( const QString &subject );

    int transportId() const;
    void setTransportId( int tid );

  private:
    class Private;
    Private *const d;
};

} // namespace MessageComposer

#endif // MESSAGECOMPOSER_INFOPART_H

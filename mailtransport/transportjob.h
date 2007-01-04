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

#ifndef KPIM_TRANSPORTJOB_H
#define KPIM_TRANSPORTJOB_H

#include <mailtransport/mailtransport_export.h>
#include <kcompositejob.h>
#include <QStringList>

class QBuffer;

namespace KPIM {

class Transport;

/**
  Abstract base class for all mail transport jobs.
  This is a job that is supposed to send exactly one mail.
*/
class MAILTRANSPORT_EXPORT TransportJob : public KCompositeJob
{
  public:
    /**
      Creates a new mail transport job.
      @param transport The transport configuration.
      @param parent The parent object.
    */
    TransportJob( Transport* transport, QObject* parent = 0 );

    /**
      Deletes this transport job.
    */
    virtual ~TransportJob();

    /**
      Sets the sender of the mail.
    */
    void setSender( const QString &sender );

    /**
      Sets the "To" receiver(s) of the mail.
    */
    void setTo( const QStringList &to );

    /**
      Sets the "Cc" receiver(s) of the mail.
    */
    void setCc( const QStringList &cc );

    /**
      Sets the "Bcc" receiver(s) of the mail.
    */
    void setBcc( const QStringList &bcc );

    /**
      Sets the content of the mail.
    */
    void setData( const QByteArray &data );

    virtual void start();

  protected:
    /**
      Returns the Transport object containing the mail transport settings.
    */
    Transport* transport() const;

    /**
      Returns the sender of the mail.
    */
    QString sender() const;

    /**
      Returns the "To" receiver(s) of the mail.
    */
    QStringList to() const;

    /**
      Returns the "Cc" receiver(s) of the mail.
    */
    QStringList cc() const;

    /**
      Returns the "Bcc" receiver(s) of the mail.
    */
    QStringList bcc() const;

    /**
      Returns the data of the mail.
    */
    QByteArray data() const;

    /**
      Returns a QBuffer opened on the message data. This is useful for
      processing the data in smaller chunks.
    */
    QBuffer* buffer();

    /**
      Do the actual work, implement in your subclass.
    */
    virtual void doStart() = 0;

  private:
    class Private;
    Private* const d;
};


}

#endif

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

#ifndef MESSAGECOMPOSER_CONTENTJOB_H
#define MESSAGECOMPOSER_CONTENTJOB_H

#include "job.h"
#include "messagecomposer_export.h"

namespace KMime {
  namespace Headers {
    class ContentTransferEncoding;
    class ContentType;
  }
}

namespace MessageComposer {

class ContentJobPrivate;

/**
*/
class MESSAGECOMPOSER_EXPORT ContentJob : public Job
{
  Q_OBJECT

  public:
    ContentJob( QObject *parent = 0 );
    virtual ~ContentJob();

    QByteArray data() const;
    void setData( const QByteArray &data );

    /// created on first call. delete them if you don't use the content
    KMime::Headers::ContentTransferEncoding *contentTransferEncoding();
    KMime::Headers::ContentType *contentType();

  protected Q_SLOTS:
    virtual void process();

  private:
    Q_DECLARE_PRIVATE( ContentJob )
};

}

#endif

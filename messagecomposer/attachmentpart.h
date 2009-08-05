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

#ifndef MESSAGECOMPOSER_ATTACHMENTPART_H
#define MESSAGECOMPOSER_ATTACHMENTPART_H

#include "messagepart.h"

#include <QtCore/QList>

#include <kmime/kmime_headers.h>

class KUrl;

namespace MessageComposer {

class MESSAGECOMPOSER_EXPORT AttachmentPart : public MessagePart
{
  Q_OBJECT

  public:
    typedef QList<AttachmentPart*> List;

    explicit AttachmentPart( QObject *parent = 0 );
    virtual ~AttachmentPart();

    /// the name= in Content-Type
    QString name() const;
    void setName( const QString &name );
    /// the filename= in Content-Disposition
    QString fileName() const;
    void setFileName( const QString &name );
    QString description() const;
    void setDescription( const QString &description );
    // otherwise "attachment"
    bool isInline() const;  // Perhaps rename to autoDisplay, since the users of
                            // this class aren't supposed to know MIME?
    void setInline( bool inl );
    // default true
    bool isAutoEncoding() const;
    void setAutoEncoding( bool enabled );
    // only if isAutoEncoding false
    KMime::Headers::contentEncoding encoding() const;
    void setEncoding( KMime::Headers::contentEncoding encoding );
    QByteArray mimeType() const;
    void setMimeType( const QByteArray &mimeType );
    bool isCompressed() const;
    void setCompressed( bool compressed );
    bool isEncrypted() const;
    void setEncrypted( bool encrypted );
    bool isSigned() const;
    void setSigned( bool sign );
    QByteArray data() const;
    void setData( const QByteArray &data );
    qint64 size() const;

    // TODO outlook-compatible names...

  private:
    class Private;
    Private *const d;
};

} // namespace MessageComposer

#endif

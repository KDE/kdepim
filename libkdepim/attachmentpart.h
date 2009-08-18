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

#ifndef KDEPIM_ATTACHMENTPART_H
#define KDEPIM_ATTACHMENTPART_H

#include "kdepim_export.h"

#include <QtCore/QList>
#include <QtCore/QMetaType>

#include <kmime/kmime_headers.h>

namespace boost {
  template <typename T> class shared_ptr;
}

namespace KPIM {

class KDEPIM_EXPORT AttachmentPart
{
  public:
    //typedef QList<AttachmentPart*> List;
    typedef boost::shared_ptr<AttachmentPart> Ptr;
    typedef QList<Ptr> List;

    AttachmentPart();
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
    QByteArray charset() const;
    void setCharset( const QByteArray &charset );
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

// FIXME I don't understand why this doesn't work if I put it outside namespace KPIM.
KDEPIM_EXPORT uint qHash( const boost::shared_ptr<KPIM::AttachmentPart> &ptr );

} // namespace KPIM

Q_DECLARE_METATYPE( KPIM::AttachmentPart::Ptr )

#endif

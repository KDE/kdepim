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

#ifndef MESSAGECORE_ATTACHMENTPART_H
#define MESSAGECORE_ATTACHMENTPART_H

#include "messagecore_export.h"

#include <kmime/kmime_headers.h>

#include <QtCore/QList>
#include <QtCore/QMetaType>

#include <boost/shared_ptr.hpp>

namespace MessageCore {

/**
 * @short A class that encapsulates an attachment.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 */
class MESSAGECORE_EXPORT AttachmentPart
{
public:
    /**
     * Defines a pointer to an attachment object.
     */
    typedef boost::shared_ptr<AttachmentPart> Ptr;

    /**
     * Defines a list of pointers to attachment objects.
     */
    typedef QList<Ptr> List;

    /**
     * Creates a new attachment part.
     */
    AttachmentPart();

    /**
     * Destroys the attachment part.
     */
    virtual ~AttachmentPart();

    /**
     * Sets the @p name of the attachment.
     *
     * The name will be used in the 'name=' part of
     * the Content-Type header.
     */
    void setName( const QString &name );

    /**
     * Returns the name of the attachment.
     */
    QString name() const;

    /**
     * Sets the file @p name of the attachment.
     *
     * The name will be used in the 'filename=' part of
     * the Content-Disposition header.
     */
    void setFileName( const QString &name );

    /**
     * Returns the file name of the attachment.
     */
    QString fileName() const;

    /**
     * Sets the @p description of the attachment.
     */
    void setDescription( const QString &description );

    /**
     * Returns the description of the attachment.
     */
    QString description() const;

    /**
     * Sets whether the attachment will be displayed inline the message.
     */
    void setInline( bool value );

    /**
     * Returns whether the attachment will be displayed inline the message.
     */
    bool isInline() const;

    /**
     * Sets whether encoding of the attachment will be determined automatically.
     */
    void setAutoEncoding( bool enabled );

    /**
     * Returns whether encoding of the attachment will be determined automatically.
     */
    bool isAutoEncoding() const;

    /**
     * Sets the @p encoding that will be used for the attachment.
     *
     * @note only applies if isAutoEncoding is @c false
     */
    void setEncoding( KMime::Headers::contentEncoding encoding );

    /**
     * Returns the encoding that will be used for the attachment.
     */
    KMime::Headers::contentEncoding encoding() const;

    /**
     * Sets the @p charset that will be used for the attachment.
     */
    void setCharset( const QByteArray &charset );

    /**
     * Returns the charset that will be used for the attachment.
     */
    QByteArray charset() const;

    /**
     * Sets the @p mimeType of the attachment.
     */
    void setMimeType( const QByteArray &mimeType );

    /**
     * Returns the mime type of the attachment.
     */
    QByteArray mimeType() const;

    /**
     * Sets whether the attachment is @p compressed.
     */
    void setCompressed( bool compressed );

    /**
     * Returns whether the attachment is compressed.
     */
    bool isCompressed() const;

    /**
     * Sets whether the attachment is @p encrypted.
     */
    void setEncrypted( bool encrypted );

    /**
     * Returns whether the attachment is encrypted.
     */
    bool isEncrypted() const;

    /**
     * Sets whether the attachment is @p signed.
     */
    void setSigned( bool sign );

    /**
     * Returns whether the attachment is signed.
     */
    bool isSigned() const;

    /**
     * Sets the payload @p data of the attachment.
     */
    void setData( const QByteArray &data );

    /**
     * Returns the payload data of the attachment.
     */
    QByteArray data() const;

    /**
     * Returns the size of the attachment.
     */
    qint64 size() const;

    /**
     * Returns whether the specified attachment part is an encapsulated message
     * (message/rfc822) or a collection of encapsulated messages (multipart/digest)
     */
    bool isMessageOrMessageCollection() const;
private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

// FIXME I don't understand why this doesn't work if I put it outside namespace KPIM.
MESSAGECORE_EXPORT uint qHash( const boost::shared_ptr<MessageCore::AttachmentPart>& );

}

Q_DECLARE_METATYPE( MessageCore::AttachmentPart::Ptr )

#endif

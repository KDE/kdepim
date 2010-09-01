/*
    This file is part of libkcal.

    Copyright (c) 2002 Michael Brade <brade@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_ATTACHMENT_H
#define KCAL_ATTACHMENT_H

#include "listbase.h"
#include "libkcal_export.h"

#include <tqstring.h>

namespace KCal {

/**
  This class represents information related to an attachment.
*/
class KDE_EXPORT Attachment
{
  public:
    typedef ListBase<Attachment> List;

    /**
      Create a Reference to some URI by copying an existing Attachment.

      @param attachment the attachment to be duplicated
    */
    Attachment( const Attachment &attachment );

    /**
      Create a Reference to some URI.

      @param uri the uri this attachment refers to
      @param mime the mime type of the resource being linked to
    */
    Attachment( const TQString &uri, const TQString &mime = TQString::null );

    /**
      Create a binary attachment.

      @param base64 the attachment in base64 format
      @param mime the mime type of the attachment
    */
    Attachment( const char *base64, const TQString &mime = TQString::null );

    ~Attachment();

    /* The VALUE parameter in iCal */
    bool isUri() const;
    TQString uri() const;
    void setUri( const TQString &uri );

    bool isBinary() const;
    char *data() const;
    void setData( const char *base64 );

    void setDecodedData( const TQByteArray &data );
    TQByteArray &decodedData();

    uint size();

    /* The optional FMTTYPE parameter in iCal */
    TQString mimeType() const;
    void setMimeType( const TQString &mime );

    /* The custom X-CONTENT-DISPOSITION parameter, used by OGo etc. */
    bool showInline() const;
    void setShowInline( bool showinline );

    /* The custom X-LABEL parameter to show a human-readable title */
    TQString label() const;
    void setLabel( const TQString &label );

    /**
      Sets the attachment "local" option, which is derived from the
      Calendar Incidence @b X-KONTACT-TYPE parameter.

      @param local is the flag to set (true) or unset (false) for the
      attachment "local" option.

      @see local()
    */
    void setLocal( bool local );

    /**
      Returns the attachment "local" flag.
    */
    bool isLocal() const;

  private:
    TQByteArray mDataCache;
    uint mSize;
    TQString mMimeType;
    TQString mUri;
    char *mData;
    TQString mLabel;
    bool mBinary;
    bool mLocal;
    bool mShowInline;

    class Private;
    Private *d;
};

}

#endif

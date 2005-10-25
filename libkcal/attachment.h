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

#include <q3cstring.h>
#include <qstring.h>


namespace KCal {

/**
  This class represents information related to an attachment.
*/
class KDE_EXPORT Attachment
{
  public:
    /** 
      Type for a list of attachements, since most documents will
      have one-or-more attachements. 
    */
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
    Attachment( const QString &uri, const QString &mime = QString::null );

    /**
      Create a binary attachment.
     
      @param base64 the attachment in base64 format
      @param mime the mime type of the attachment
    */
    Attachment( const char *base64, const QString &mime = QString::null );
    ~Attachment();

    /** The VALUE parameter in iCal may represent a URI for the attachment. */
    QString uri() const;
    /** Is the VALUE parameter one that represents a URI? */
    bool isUri() const;
    /** Sets the VALUE parameter for the attachment to the given URI.
        @param uri The URI to use for this attachment.
    */
    void setUri( const QString &uri );
    
    /* Data attachments are currently not implemented in iCal. */
    bool isBinary() const;
    char *data() const;
    QByteArray &decodedData() const;
    void setData( const char *base64 );
    void setDecodedData( const QByteArray &data );
    /* size only for binary attachments */
    uint size() const;

    /* The optional FMTTYPE parameter in iCal */
    QString mimeType() const;
    void setMimeType( const QString &mime );
		
		/* The custom X-CONTENT-DISPOSITION parameter, used by OGo etc. */
		bool showInline() const;
		void setShowInline( bool showinline );
		
		/* The custom X-LABEL parameter to show a human-readable title */
		QString label() const;
		void setLabel( const QString &label );
		
		/* The custom X-KONTACT-TYPE parameter, controls whether the attachment
		   is 'local', e.g. with hidden path. */
		bool isLocal() const;
		void setLocal( bool local );

  private:
    QString mMimeType;
    QString mData;
    bool mBinary;
		bool mShowInline;
		bool mLocal;
		QString mLabel;

    class Private;
    Private *d;
};

}

#endif

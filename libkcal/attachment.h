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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_ATTACHMENT_H
#define KCAL_ATTACHMENT_H

#include "listbase.h"
#include "libkcal_export.h"

#include <qstring.h>

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
    Attachment( const QString &uri, const QString &mime = QString::null );

    /**
      Create a binary attachment.
     
      @param base64 the attachment in base64 format
      @param mime the mime type of the attachment
    */
    Attachment( const char *base64, const QString &mime = QString::null );

    /* The VALUE parameter in iCal */
    bool isUri() const;
    QString uri() const;
    void setUri( const QString &uri );
    
    bool isBinary() const;
    char *data() const;
    void setData( const char *base64 );

    /* The optional FMTTYPE parameter in iCal */
    QString mimeType() const;
    void setMimeType( const QString &mime );
		
		/* The custom X-CONTENT-DISPOSITION parameter, used by OGo etc. */
		bool showInline() const;
		void setShowInline( bool showinline );
		
		/* The custom X-LABEL parameter to show a human-readable title */
		QString label() const;
		void setLabel( const QString &label );

  private:
    QString mMimeType;
    QString mData;
    bool mBinary;
		bool mShowInline;
		QString mLabel;

    class Private;
    Private *d;
};

}

#endif

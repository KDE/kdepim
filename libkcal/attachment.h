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

#ifndef _ATTACHMENT_H
#define _ATTACHMENT_H

#include <qstring.h>


namespace KCal {

/**
 * This class represents information related to an attachment.
 */
class Attachment
{
public:
    /**
     * Create a Reference to some URL.
     * @param url the url this attachment refers to
     */
    Attachment(const QString& url);

    /**
     * Create a binary attachment.
     * @param base64 the attachment in base64 format
     */
    Attachment(const char *base64);

    /** Destruct Attachment */
    virtual ~Attachment();

    bool isURL() const;
    QString url() const;
    void setURL(const QString& url);
    
    bool isBinary() const;
    /** @return the content as base64 */
    const char *data() const;
    void setData(const char *base64);

    /* The FMTTYPE parameter in iCal */
    QString mimeType() const;
    void setMimeType(const QString& mime);
private:
    QString mMimeType;
    QString mData;
};

}

#endif

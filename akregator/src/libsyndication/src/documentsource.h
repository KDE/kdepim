/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef LIBSYNDICATION_DOCUMENTSOURCE_H
#define LIBSYNDICATION_DOCUMENTSOURCE_H

#include <ksharedptr.h>

class QByteArray;
class QDomDocument;

namespace LibSyndication {

/** 
 * Represents the source of a syndication document, as read from the
 * downloaded file.
 *
 * It provides a (cached) DOM representation of the document, but keeps
 * the raw data available (for (rarely used) non-XML formats like Okay!
 * News...).
 *
 * This way the document can be passed to all available parsers (to find the
 * right one for the source), regardless whether they parse XML formats or
 * non-XML formats, without having every parser to do the XML parsing again.
 *
 * @author Frank Osterfeld
 */
class DocumentSource
{
    public:

        /**
         * Creates an empty document source. The raw representation is empty and 
         * the DOM representation will be invalid.
         */
        DocumentSource();
        
        /**
         * Creates a DocumentSource object from a raw byte array
         * 
         * @param source the raw source (of the downloaded feed file usually)
         */
        DocumentSource(const QByteArray& source);
        
        /** 
         * Copy constructor. The d pointer is shared, so this is a cheap
         * operation.
         * 
         * @param other DocumentSource to copy
         */
        DocumentSource(const DocumentSource& other);

        virtual ~DocumentSource();

        /**
         * Assignment operator. The d pointer is shared, so this is a cheap
         * operation.
         * 
         * @param other DocumentSource to assign to this instance
         * @return reference to this instance
         */
        DocumentSource& operator=(const DocumentSource& other);

        /**
         * Returns the feed source as byte array.
         *
         * @return the feed source as raw byte array.
         */
        QByteArray asByteArray() const;

        /**
         * Returns the feed source as DOM document.
         * The document is parsed only on the first call of this method
         * and then cached.
         *
         * @return XML representation parsed from the raw source
         */
        QDomDocument asDomDocument() const;

    private: 

        class DocumentSourcePrivate;
        KSharedPtr<DocumentSourcePrivate> d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DOCUMENTSOURCE_H

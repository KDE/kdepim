/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#ifndef LIBSYNDICATION_ATOM_CONTENT_H
#define LIBSYNDICATION_ATOM_CONTENT_H

#include <libsyndication/elementwrapper.h>
#include <libsyndication/sharedptr.h>

#include <QtCore/QString>

class QByteArray;
class QDomElement;

namespace Syndication {
namespace Atom {

/**
 * The content element either contains or links the content of an entry.
 * The content is usually plain text or HTML, but arbitrary XML or binary
 * content are also possible. If isContained() is false, the content is
 * not contained in the feed source, but linked.
 * 
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Content : public ElementWrapper
{
    public:

        /**
         * format of the content.
         */
        enum Format
        {
            PlainText, /**< the content is plain text (i.e. "<", ">"
                        * etc. are text, not
                   * markup */
            EscapedHTML, /**< the content is escaped HTML, (i.e., "<", ">" etc.
                          * are markup) */
            XML, /**< the content is embedded XML */
            Binary /**< the content is base64-encoded binary content */
        };
       
        /**
         * maps a mimetype to Format enum according to the Atom 1.0
         * specification
         *
         * @param type a valid mimetype, or one of "text", "html", "xhtml"
         * @param src content source, see src() for details.
         *
         */
        static Format mapTypeToFormat(const QString& type,
                                      const QString& src=QString());
        
        /**
         * creates a null content object.
         */
        Content();
        
        /**
         * creates a Content object wrapping an atom:content element.
         * @param element a DOM element, should be a atom:content element
         * (although not enforced), otherwise this object will not parse 
         * anything useful
         */
        Content(const QDomElement& element);

        /**
         * creates a copy of another Content object
         * 
         * @param other the content object to copy
         */
        Content(const Content& other);
        
        /**
         * destructor
         */
        ~Content();
        
        /**
         * assigns another content objecct
         * 
         * @param other the Content object to assign
         */
        Content& operator=(const Content& other);
        
        /**
         * the type of the content. This is either "text" (plain text), 
         * "html" (escaped HTML), "xhtml" (embedded XHTML) or a mime type
         * 
         * @return the content type. If no type is specified, "text" (the
         * default) is returned.
         */
        QString type() const;

        /**
         * If src() is set, the content of this entry is not contained in the
         * feed source, but available on the net.
         * src() then contains a URL (more precise: IRI reference) linking to
         * remote content.
         * If src is provided, type() should contain a mimetype, instead of "text",
         * "html" or "xhtml".
         * 
         * @return  a null string if the content is contained in the feed
         * source, or a URL linking to the remote content
         */
        QString src() const;
        
        /**
         * returns the content as string. If the content format is Text, the
         * returned string contains the text as HTML.
         * If the content is embedded XML, the XML is returned as string.
         * 
         * @return a string representation of the content, or a null string if
         * the content is either binary content or not contained but linked
         * (see isContained())
         */
      
        QString asString() const;
        
        /**
         * returns binary content as byte array.
         * 
         * @return byte array containing the embedded binary data, or
         * an empty array if the content is not in binary format
         */
        QByteArray asByteArray() const;
        
        /**
         * returns the content format
         */
        Format format() const;
        
        /**
         * returns whether the content is contained in the feed source,
         * or not. If it is not contained, src() provides a URL to the
         * content.
         */
        bool isContained() const;
        
        /**
         * returns whether the content is embedded XML.
         * Use element() to access the DOM tree, or asString() to get the XML
         * as string.
         */
        bool isXML() const;
        
        /**
         * returns whether the content is binary content or not.
         * Use asByteArray() to access it.
         */
        bool isBinary() const;
        
        /**
         * returns whether the content is plain text or not.
         * Use asString() to access it.
         */
        bool isPlainText() const;
        
        /**
         * returns whether the content is escaped HTML or not
         * Use asString() to access it
         */
        bool isEscapedHTML() const;
        
        /**
         * returns a description of the content object
         * for debugging purposes
         * 
         * @return debug string
         */
        QString debugInfo() const;
        
    private:
        class ContentPrivate;
        SharedPtr<ContentPrivate> d;
};

} // namespace Atom
} // namespace Syndication

#endif // LIBSYNDICATION_ATOM_CONTENT_H

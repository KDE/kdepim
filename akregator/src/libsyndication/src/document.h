/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBSYNDICATION_DOCUMENT_H
#define LIBSYNDICATION_DOCUMENT_H

class QString;

namespace LibSyndication {

class DocumentVisitor;

/**
 * Document interface. A document is a representation parsed from a document 
 * source (see DocumentSource). It typically contains several information items
 * (i.e. articles) plus some additional metadata about the news feed.
 * The Document classes from the several syndication formats must implement
 * this interface. It's main purpose is to provide access for document visitors
 * (see DocumentVisitor).
 *
 * @author Frank Osterfeld
 */
class Document
{
    public:

        virtual ~Document() {}

        virtual bool accept(DocumentVisitor* visitor) = 0;

        /**
         * Returns a description of the document for debugging purposes.
         *
         * @return debug string
         */
        virtual QString debugInfo() const = 0;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DOCUMENT_H


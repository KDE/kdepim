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

#ifndef LIBSYNDICATION_DOCUMENTVISITOR_H
#define LIBSYNDICATION_DOCUMENTVISITOR_H

namespace LibSyndication {

class Document;

namespace RSS2 
{
    class Document;
}

/**
 * Visitor interface, following the Visitor design pattern. Use this if you
 * want to process documents and the way how to handle the document depends
 * on it's concrete type (e.g. RSS2::Document, Atom::Document...).
 *
 * TODO: insert code example
 *
 * @author Frank Osterfeld
 */
class DocumentVisitor
{
    public:

        virtual ~DocumentVisitor() {}

        virtual bool visit(Document* document);

        virtual bool visit(LibSyndication::RSS2::Document* document) { return false; }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DOCUMENTVISITOR_H

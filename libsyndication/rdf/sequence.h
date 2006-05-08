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
#ifndef LIBSYNDICATION_RDF_SEQUENCE_H
#define LIBSYNDICATION_RDF_SEQUENCE_H

#include <libsyndication/rdf/resource.h>

#include <libsyndication/sharedptr.h>

template <class T> class QList;

namespace Syndication {
namespace RDF {


class Sequence;
typedef SharedPtr<Sequence> SequencePtr;

/**
 * Sequence container, a sequence contains an ordered list
 * of RDF nodes. (opposed to the usually unordered graph
 * structure)
 */
class SYNDICATION_EXPORT Sequence : public Resource
{
    public:
        
        Sequence();
        Sequence(const QString& uri);
        Sequence(const Sequence& other);
        virtual ~Sequence();
        
        virtual Sequence& operator=(const Sequence& other);
        
        virtual void accept(NodeVisitor* visitor, NodePtr ptr);
        
        virtual Sequence* clone() const;
        
        virtual void append(NodePtr node);
        virtual QList<NodePtr> items() const;
        virtual bool isSequence() const;
        
    private:
        
        class SequencePrivate;
        SharedPtr<SequencePrivate> d;

};

} // namespace RDF
} // namespace Syndication

#endif // LIBSYNDICATION_RDF_SEQUENCE_H

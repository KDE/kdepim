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
#ifndef LIBSYNDICATION_RDF_NODE_H
#define LIBSYNDICATION_RDF_NODE_H

#include <kdepim_export.h>

#include <libsyndication/sharedptr.h>

namespace Syndication {
namespace RDF {

class Model;
class Node;
class NodeVisitor;

typedef SharedPtr<Node> NodePtr;

class SYNDICATION_EXPORT Node
{
    public:
        
        virtual ~Node();
        
        virtual void accept(NodeVisitor* visitor, NodePtr ptr);
        
        virtual bool operator==(const Node& other) const = 0;
        
        /**
         * returns a copy of the object. Must be implemented
         * by subclasses to return a copy using the concrete
         * type
         */
        virtual Node* clone() const = 0;
        
        /**
         * returns whether this node is a null node
         */
        virtual bool isNull() const = 0;

        /**
         * returns whether this node is a resource
         */
        virtual bool isResource() const = 0;
        
        /**
         * returns whether this node is a property
         */
        virtual bool isProperty() const = 0;

        /**
         * returns whether this node is a literal
         */
        virtual bool isLiteral() const = 0;
        
        /**
         * returns whether this node is an RDF sequence
         */
        virtual bool isSequence() const = 0;
        
        /**
         * returns whether this node is an anonymous resource
         */
        virtual bool isAnon() const = 0;
        
        virtual unsigned int id() const = 0;
        
        /**
         * used in Model
         * @internal
         */
        virtual void setModel(const Model& model) = 0;
        
        /**
         *  used in Model
         * @internal
         */
        virtual void setId(unsigned int id) = 0;
        
    protected:

        /** 
         * used to generate unique IDs for node objects 
         */
        static unsigned int idCounter;
};

} // namespace RDF
} // namespace Syndication

#endif // LIBSYNDICATION_RDF_NODE_H

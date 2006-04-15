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
#ifndef LIBSYNDICATION_RDF_MODEL_H
#define LIBSYNDICATION_RDF_MODEL_H

#include <libsyndication/rdf/literal.h>
#include <libsyndication/rdf/node.h>
#include <libsyndication/rdf/property.h>
#include <libsyndication/rdf/sequence.h>
#include <libsyndication/rdf/statement.h>
#include <libsyndication/sharedptr.h>

#include <kdepimmacros.h>

#include <QtCore/QString>

template <class T> class QList;

namespace Syndication {
namespace RDF {

/**
 * An RDF model, a set of RDF statements.
 * Model objects are implicitely shared.
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Model
{
    public:

        /**
         * default constructor, creates an empty model
         * containing no statements 
         */
        Model();
        
        /**
         * constructs a model from another.
         * Both models will share the same set of statements,
         * so adding/removing statements from one model also
         * modifies the other!
         * 
         * @param other another model
         */
        Model(const Model& other);

        virtual ~Model();

        /**
         * assigns another model. Both models will share the same 
         * set of statements, so adding/removing statements from 
         * one model also modifies the other!
         * 
         * @param other another model
         */
        Model& operator=(const Model& other);
        
        /**
         * Returns whether two models objects represent the same model
         * (i.e. share the same underlying statement set). Currently this
         * method does _not_ compare the statement list.
         * Two indepently created models containing the same statements
         * are not equal!
         */
        bool operator==(const Model& other) const;
        
        /**
         * creates a resource and associates it with this model. If the model
         * already contains a resource with the given URI, the existing instance
         * is returned.
         * 
         * @param uri the URI of the resource. If QString::null, a blank node 
         * is created.
         * @return a shared pointer to the requested resource
         */
        virtual ResourcePtr createResource(const QString& uri=QString());
        
        /**
         * creates a property and associates it with this model. If the model
         * already contains a property with the given URI, the existing instance
         * is returned.
         * 
         * @param uri the URI of the property. This must be non-empty, otherwise
         * null property is returned
         * @return a shared pointer to the requested property
         */
        virtual PropertyPtr createProperty(const QString& uri);

        /**
         * creates a sequence and associates it with this model. If the model
         * already contains a sequence with the given URI, the existing
         * instance is returned.
         * 
         * @param uri the URI of the sequence, or a null string for an
         * anonymous instance
         * @return a shared pointer to the requested sequence
         */
        virtual SequencePtr createSequence(const QString& uri=QString());

        /**
         * creates a literal and associates it with this model. 
         * 
         * @param uri the literal text
         * @return a shared pointer to the requested literal
         */
        virtual LiteralPtr createLiteral(const QString& text);
        
        /**
         * adds a statement to the model.
         * 
         * @param subject
         * @param predicate
         * @param object
         * @return a shared pointer to a statement associated with this
         * model, with the given @c subject, @c predicate and @c object
         */
        virtual StatementPtr addStatement(ResourcePtr subject, 
                                          PropertyPtr predicate,
                                          NodePtr object);
        
        /**
         * removes a statement from the model.
         * 
         * @param subject subject of the statement
         * @param predicate predicate of the statement
         * @param object object of the statement
         */
        virtual void removeStatement(ResourcePtr subject,
                                     PropertyPtr predicate,
                                     NodePtr object);
        
        /**
         * removes a statement from the model.
         * 
         * @param statement the statement to remove
         */
        virtual void removeStatement(StatementPtr statement);
                
        /**
         * returns whether this model is empty, i.e. contains no statements.
         */
        virtual bool isEmpty() const;

        /**
         * returns all resources of a given type.
         * subClassOf semantics are ignored.
         * 
         * @param type a resource representing an RDFS class
         */
        virtual QList<ResourcePtr> resourcesWithType(ResourcePtr type) const;

        /**
         * returns a list of the statements in this model.
         * 
         */
        virtual QList<StatementPtr> statements() const;

        /**
         * searches the model for a node by ID.
         *
         * @param id the ID to search for
         * @return the node with the given ID, or a null node (which is of type
         * Literal) if the model doesn't contain the node with this ID
         */
        virtual NodePtr nodeByID(uint id) const;
        
        /**
         * searches the model for a resource by ID.
         *
         * @param id the ID to search for
         * @return the resource with the given ID, or a null resource if the 
         * model doesn't contain a resource with this ID
         */
        virtual ResourcePtr resourceByID(uint id) const;
        
        /**
         * searches the model for a property by ID.
         *
         * @param id the ID to search for
         * @return the property with the given ID, or a null property if the
         * model doesn't contain a property with this ID
         */
        virtual PropertyPtr propertyByID(uint id) const;
        
        /**
         * searches the model for a literal by ID.
         *
         * @param id the ID to search for
         * @return the literal with the given ID, or a null literal if the
         * model doesn't contain a literal with this ID
         */
        virtual LiteralPtr literalByID(uint id) const;

        /**
         * @internal
         */
        virtual bool resourceHasProperty(const Resource* resource,
                                         PropertyPtr property) const;

        /**
         * @internal
         */
        virtual StatementPtr resourceProperty(const Resource* resource,
                                              PropertyPtr property) const;

        /**
         * @internal
         */
        virtual QList<StatementPtr> resourceProperties(const Resource* resource,
                                              PropertyPtr property) const;

        /**
         * a debug string listing the contained statements for 
         * debugging purposes
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;

    private:
        class ModelPrivate;
        SharedPtr<ModelPrivate> d;
};


} // namespace RDF
} // namespace Syndication

#endif // LIBSYNDICATION_RDF_MODEL_H

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

#include "document.h"
#include "model.h"
#include "modelmaker.h"
#include "parser.h"
#include "rdfvocab.h"
#include "rssvocab.h"

#include "../documentsource.h"

#include <QDomDocument>
#include <QDomNodeList>
#include <QList>
#include <QString>

#include "property.h"
#include "resource.h"
#include "statement.h"
#include <iostream>

namespace LibSyndication {
namespace RDF {

bool Parser::accept(const DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    
    if (doc.isNull())
        return false;
    
    return doc.documentElement().namespaceURI() == RDFVocab::self()->namespaceURI();
}

Document* Parser::parse(const DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    
    if (doc.isNull())
        return 0;
    
    ModelMaker maker;
    Model model = maker.createFromXML(doc);
    
    QList<ResourcePtr> channels = model.resourcesWithType(RSSVocab::self()->channel());
    
    QList<StatementPtr> stmts = model.statements();
    QList<StatementPtr>::ConstIterator it = stmts.begin();
    QList<StatementPtr>::ConstIterator end = stmts.end();
    for ( ; it != end; ++it)
    {
        std::cout << "# " << (*it)->subject()->uri().toLocal8Bit().data() << (*it)->predicate()->uri().toLocal8Bit().data() << (*it)->object()->id() << std::endl;
    }
    std::cout << "#foob " << std::endl;

    if (channels.isEmpty())
        return 0;
  
    return new Document(*(channels.begin()));
}

QString Parser::format() const
{
    return QString::fromLatin1("rdf");
}

        
} // namespace RDF
} // namespace LibSyndication

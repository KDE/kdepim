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
#include "literal.h"
#include "node.h"
#include "nodevisitor.h"
#include "property.h"
#include "resource.h"
#include "sequence.h"

namespace Syndication {
namespace RDF {

NodeVisitor::~NodeVisitor() {}

void NodeVisitor::visit(NodePtr node)
{
    node->accept(this, node);
}
    
bool NodeVisitor::visitLiteral(LiteralPtr)
{
    return false;
}
        
bool NodeVisitor::visitNode(NodePtr)
{
    return false;
}

bool NodeVisitor::visitProperty(PropertyPtr)
{
    return false;
}
        
bool NodeVisitor::visitResource(ResourcePtr)
{
    return false;
}
        
bool NodeVisitor::visitSequence(SequencePtr)
{
    return false;
}
} // namespace RDF
} // namespace Syndication

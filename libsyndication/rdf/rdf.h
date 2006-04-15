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
#ifndef LIBSYNDICATION_RDF_RDF_H
#define LIBSYNDICATION_RDF_RDF_H

#include <libsyndication/rdf/contentvocab.h>
#include <libsyndication/rdf/document.h>
#include <libsyndication/rdf/dublincore.h>
#include <libsyndication/rdf/dublincorevocab.h>
#include <libsyndication/rdf/image.h>
#include <libsyndication/rdf/item.h>
#include <libsyndication/rdf/literal.h>
#include <libsyndication/rdf/model.h>
#include <libsyndication/rdf/modelmaker.h>
#include <libsyndication/rdf/node.h>
#include <libsyndication/rdf/nodevisitor.h>
#include <libsyndication/rdf/parser.h>
#include <libsyndication/rdf/property.h>
#include <libsyndication/rdf/rdfvocab.h>
#include <libsyndication/rdf/resource.h>
#include <libsyndication/rdf/resourcewrapper.h>
#include <libsyndication/rdf/rssvocab.h>
#include <libsyndication/rdf/sequence.h>
#include <libsyndication/rdf/statement.h>
#include <libsyndication/rdf/syndication.h>
#include <libsyndication/rdf/syndicationvocab.h>
#include <libsyndication/rdf/textinput.h>

namespace Syndication {

/** 
 * LibSyndication's parser for the RDF-based 
 * RSS 0.9 and RSS 1.0 formats
 */
namespace RDF {}

}

#endif // LIBSYNDICATION_RDF_RDF_H

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

#include "model.h"
#include "property.h"
#include "rdfvocab.h"

#include <QString>

namespace LibSyndication {
namespace RDF {

Resource RDFVocab::seq()
{
    return Resource(QString::fromLatin1("http://www.w3.org/1999/02/22-rdf-syntax-ns#Seq"), Model());
}

Property RDFVocab::type()
{
    return Property(QString::fromLatin1("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
}

Property RDFVocab::li()
{
    return Property(QString::fromLatin1("http://www.w3.org/1999/02/22-rdf-syntax-ns#li"));
}

QString RDFVocab::namespaceURI()
{
    return QString::fromLatin1("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
}

} // namespace RDF
} // namespace LibSyndication

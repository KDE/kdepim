/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "utils.h"

/** In a document doc with node node, add an element with name ns and tagname tag. Return the new element 
 */
QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag )
{
  QDomElement el = doc.createElementNS( ns, tag );
  node.appendChild( el );
  return el;
}

/**
 In a document doc with node node, add an element with namespace ns and tagname tag. Add a textnode in
 the element with text contents text. Return the new element.
 */
QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag, const QString& text )
{
  QDomElement el = doc.createElementNS( ns, tag );
  QDomText textnode = doc.createTextNode( text );
  el.appendChild( textnode );
  node.appendChild( el );
  return el;
}


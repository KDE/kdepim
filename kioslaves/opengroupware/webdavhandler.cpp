/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <config.h>

#include "webdavhandler.h"

#include <limits.h>

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>
#include <kconfig.h>

#include <qfile.h>


WebdavHandler::WebdavHandler()
{
}


QDomElement WebdavHandler::addElement( QDomDocument &doc, QDomNode &node,
                                       const QString &tag )
{
  QDomElement el = doc.createElement( tag );
  node.appendChild( el );
  return el;
}

QDomElement WebdavHandler::addDavElement( QDomDocument &doc, QDomNode &node,
                                          const QString &tag )
{
  QDomElement el = doc.createElementNS( "DAV", tag );
  node.appendChild( el );
  return el;
}

QDomElement WebdavHandler::addSloxElement( QDomDocument &doc, QDomNode &node,
                                           const QString &tag,
                                           const QString &text )
{
  QDomElement el = doc.createElementNS( "SLOX", tag );
  if ( !text.isEmpty() ) {
    QDomText textnode = doc.createTextNode( text );
    el.appendChild( textnode );
  }
  node.appendChild( el );
  return el;
}

QDomDocument WebdavHandler::createAllPropsRequest()
{
  QDomDocument doc;

  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement(  doc, root, "prop" );
  WebdavHandler::addDavElement(  doc, prop, "getcontentlength");
  WebdavHandler::addDavElement(  doc, prop, "getlastmodified" );
  WebdavHandler::addDavElement(  doc, prop, "displayname" );
  WebdavHandler::addDavElement(  doc, prop, "resourcetype" );
  prop.appendChild( doc.createElementNS( "http://apache.org/dav/props/", "executable" ) );
  return doc;
}

/*
    This file is part of KitchenSync.

    Copyright (c) 2006 Daniel Gollub <dgollub@suse.de> 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "xmldiffalgo.h"

#include <kdebug.h>

using namespace KSync;

#ifndef KDE_USE_FINAL
// With --enable-final, we get the (identical) compareString from
// addresseediffalgo.cpp
//
static bool compareString( const TQString &left, const TQString &right )
{
  if ( left.isEmpty() && right.isEmpty() )
    return true;
  else
    return left == right;
}
#endif

XmlDiffAlgo::XmlDiffAlgo( const TQString &leftXml, const TQString &rightXml )
{
  kdDebug() << __func__ << " " << __LINE__ << endl;

  mLeftXml.setContent( leftXml );
  mRightXml.setContent( rightXml );

}

XmlDiffAlgo::XmlDiffAlgo( const TQDomDocument &leftXml, const TQDomDocument &rightXml )
  : mLeftXml( leftXml ), mRightXml( rightXml )
{
  kdDebug() << __func__ << " " << __LINE__ << endl;
}

void XmlDiffAlgo::appendSingleNodes(TQDomElement &element, bool isLeft)
{
  TQDomNode node;

  for ( node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement child = node.toElement();

    if (isLeft)
      additionalLeftField( node.nodeName(), child.text() );
    else
      additionalRightField( node.nodeName(), child.text() );
  }

}

void XmlDiffAlgo::appendConflictNodes(TQDomElement &leftElement, TQDomElement &rightElement)
{
  TQDomNode left, right;
  TQDomElement leftChild, rightChild;

  for ( left = leftElement.firstChild(); !left.isNull(); left = left.nextSibling() ) {
    leftChild = left.toElement();

    for ( right = rightElement.firstChild(); !right.isNull(); right = right.nextSibling() ) {
      rightChild = right.toElement();

      if ( leftChild.tagName() != rightChild.tagName() )
        continue;

      if (leftChild.text().isEmpty() || rightChild.text().isEmpty())
        continue;

      TQString id = leftChild.tagName();
      if (id == "Content")
        id = left.parentNode().nodeName();

      conflictField( id, leftChild.text(), rightChild.text() );

      left.parentNode().removeChild( left );
      left = leftElement.firstChild();

      right.parentNode().removeChild( right );
      right = rightElement.firstChild();

    }
  }
}

void XmlDiffAlgo::compareNode(TQDomElement &leftElement, TQDomElement &rightElement)
{
  TQDomNode left, right;
  TQDomElement leftChild, rightChild;
  TQDomNodeList nlist;
top:;

  for ( left = leftElement.firstChild(); !left.isNull(); left = left.nextSibling() ) {
    leftChild = left.toElement();

    for ( right = rightElement.firstChild(); !right.isNull(); right = right.nextSibling() ) {
      rightChild = right.toElement();

      if (leftChild.tagName() != rightChild.tagName())
        continue;

      if ( left.childNodes().count() > 1 && right.childNodes().count() > 1 ) {
        compareNode( leftChild, rightChild );

        if ( !left.hasChildNodes() && !right.hasChildNodes() ) {
          left.parentNode().removeChild( left );
          right.parentNode().removeChild( right );
          goto top;
        }

        break;
      }

      if ( leftChild.text() == rightChild.text() ) {
        TQString id = leftChild.tagName();

        if ( id == "Content" )
          id = left.parentNode().nodeName(); 
 
	if ( id != "Type" )
          //matchingField( id, leftChild.text(), rightChild.text() );

        left.parentNode().removeChild( left );
        right.parentNode().removeChild( right );
        goto top;
      }
    }
  }

  appendConflictNodes(rightElement, leftElement);

  appendSingleNodes(rightElement, false);
  appendSingleNodes(leftElement, true);
}

void XmlDiffAlgo::run()
{
  kdDebug() << __func__ << endl;	
  begin();

  TQDomElement leftElement = mLeftXml.documentElement();
  TQDomElement rightElement = mRightXml.documentElement();

  compareNode( leftElement, rightElement );

  end();
}


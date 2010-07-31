/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
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
#ifndef PARSER_H
#define PARSER_H

#include <kode/code.h>
#include <kode/printer.h>
#include <kode/typedef.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqdom.h>
#include <tqregexp.h>
#include <tqmap.h>

#include <iostream>

class Pattern
{
  public:
    Pattern();

    bool isEmpty();

    TQString asString();

    void merge( Pattern p );
  
    bool optional;
    bool zeroOrMore;
    bool oneOrMore;
    bool choice;
};

class Reference
{
  public:
    typedef TQValueList<Reference *> List;
  
    Reference() : substituted( false ) {}
  
    TQString name;
    Pattern pattern;

    bool substituted;
};

class Attribute
{
  public:
    typedef TQValueList<Attribute *> List;
  
    TQString name;
    TQValueList<TQString> choices;
    TQString defaultValue;
    Pattern pattern;
};

class Element
{
  public:
    typedef TQValueList<Element *> List;
  
    Element();
  
    TQString name;
    Element::List elements;
    Attribute::List attributes;
    Reference::List references;
    Pattern pattern;
    bool hasText;
    bool isEmpty;
};

class Parser
{
  public:
    Parser();

    Element *parse( const TQDomElement &docElement );

    Reference *parseReference( const TQDomElement &referenceElement );
    bool parseAttribute( const TQDomElement &attributeElement,
                               Attribute *a );
    bool parseElement( const TQDomElement &elementElement, Element *e,
                       Pattern pattern );

    void substituteReferences( Element *s );

    void doIndent( int cols );

    void dumpPattern( Pattern pattern );
    void dumpReferences( const Reference::List &references,
                         int indent );
    void dumpAttributes( const Attribute::List &attributes,
                         int indent );
    void dumpElements( const Element::List &elements, int indent );
    void dumpElement( Element *element, int indent );
    void dumpTree( Element *s );
    void dumpDefinitionMap();

  private:
    TQMap<TQString,Element::List> mDefinitionMap;
};

#endif

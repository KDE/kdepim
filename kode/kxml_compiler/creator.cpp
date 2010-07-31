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

#include "creator.h"

#include <kode/code.h>
#include <kode/printer.h>
#include <kode/typedef.h>
#include <kode/statemachine.h>

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

Creator::Creator( XmlParserType p, XmlWriterType w )
  : mXmlParserType( p ), mXmlWriterType( w )
{
  setExternalClassNames();
}

void Creator::setExternalClassPrefix( const TQString &prefix )
{
  mExternalClassPrefix = prefix;

  setExternalClassNames();
}

void Creator::setExternalClassNames()
{
  mParserClass.setName( mExternalClassPrefix + "Parser" );
  mWriterClass.setName( mExternalClassPrefix + "Writer" );
}

KODE::File &Creator::file() { return mFile; }

TQString Creator::upperFirst( const TQString &str )
{
  return KODE::Style::upperFirst( str );
}

TQString Creator::lowerFirst( const TQString &str )
{
  return KODE::Style::lowerFirst( str );
}

void Creator::createProperty( KODE::Class &c, const TQString &type,
                     const TQString &name )
{
  KODE::MemberVariable v( name, type );
  c.addMemberVariable( v );

  KODE::Function mutator( "set" + upperFirst( name ), "void" );
  mutator.addArgument( "const " + type + " &v" );
  mutator.addBodyLine( v.name() + " = v;" );
  c.addFunction( mutator );

  KODE::Function accessor( name, type );
  accessor.setConst( true );
  accessor.addBodyLine( "return " + v.name() + ";" );
  c.addFunction( accessor );
}

void Creator::createElementFunctions( KODE::Class &c, Element *e )
{
  if ( e->hasText ) {
    createProperty( c, "TQString", e->name );
    if ( mXmlParserType == XmlParserCustomExternal ) {
      createTextElementParserCustom( c, e );
    }
    return;
  }

  TQString type = upperFirst( e->name );

  if ( !mFile.hasClass( type ) ) {
    createClass( e );
  }

  TQString name = lowerFirst( e->name );

  if ( e->pattern.oneOrMore || e->pattern.zeroOrMore ||
       e->pattern.choice || e->pattern.optional ) {
    registerListTypedef( type );

    c.addHeaderInclude( "tqvaluelist.h" );
    type = type + "::List";
    TQString className = upperFirst( name );
    name = name + "List";

    KODE::Function adder( "add" + className, "void" );
    adder.addArgument( className + " *v" );

    KODE::Code code;
    code += "m" + upperFirst( name ) + ".append( v );";

    adder.setBody( code );

    c.addFunction( adder );
  }

  createProperty( c, type, name );
}

void Creator::createClass( Element *element )
{
  TQString className = upperFirst( element->name );

  if ( mProcessedClasses.find( className ) != mProcessedClasses.end() ) {
    return;
  }

  KODE::Class c( className );

  mProcessedClasses.append( className );

  TQValueList<Attribute *>::ConstIterator itA;
  for( itA = element->attributes.begin();
       itA != element->attributes.end(); ++itA ) {
    Attribute *a = *itA;

    createProperty( c, "TQString", a->name );
  }

  TQValueList<Element *>::ConstIterator itE;
  for( itE = element->elements.begin(); itE != element->elements.end();
       ++itE ) {
    createElementFunctions( c, *itE );
  }

  TQValueList<Reference *>::ConstIterator itR;
  for( itR = element->references.begin(); itR != element->references.end();
       ++itR ) {
    Element e;
    e.name = (*itR)->name;
    e.pattern = (*itR)->pattern;
    createElementFunctions( c, &e );
  }

  createElementParser( c, element );
  createElementWriter( c, element );

  mFile.insertClass( c );
}

void Creator::createElementWriter( KODE::Class &c, Element *element )
{
  KODE::Function writer( "writeElement", "TQString" );
  
  KODE::Code code;
  
  code += "TQString xml;";
  
  TQString tag = "<" + element->name;

  TQValueList<Attribute *>::ConstIterator it3;
  for( it3 = element->attributes.begin(); it3 != element->attributes.end();
        ++it3 ) {
    tag += " " + (*it3)->name + "=\\\"\" + " + (*it3)->name + "() + \"\\\"";
  }

  if ( element->isEmpty ) {
    tag += "/";
  }

  tag += ">\\n";

  code += "xml += indent() + \"" + tag + "\";";

  if ( !element->isEmpty ) {
    code += "indent( 2 );";

    TQValueList<Element *>::ConstIterator it;
    for( it = element->elements.begin(); it != element->elements.end(); ++it ) {
      Element *e = *it;
      TQString type = upperFirst( e->name );
      if ( e->pattern.oneOrMore || e->pattern.zeroOrMore ) {
        code += type + "::List list = " + e->name + "List();";
        code += type + "::List::ConstIterator it;";
        code += "for( it = list.begin(); it != list.end(); ++it ) {";
        code.indent();
        code += "xml += (*it)->writeElement();";
        code.unindent();
        code += "}";
      } else {
        if ( e->hasText ) {
          code += "xml += indent() + \"<" + e->name + ">\" + " + e->name + "() + \"</" +
                  e->name + ">\\n\";";
        } else {
          code += "xml += " + type + "()->writeElement()";
        }
      }
    }

    TQValueList<Reference *>::ConstIterator it2;
    for( it2 = element->references.begin(); it2 != element->references.end();
         ++it2 ) {
      Reference *r = *it2;
      TQString type = upperFirst( r->name );
      if ( r->pattern.oneOrMore || r->pattern.zeroOrMore ) {
        code += type + "::List list2 = " + r->name + "List();";
        code += type + "::List::ConstIterator it2;";
        code += "for( it2 = list2.begin(); it2 != list2.end(); ++it2 ) {";
        code.indent();
        code += "xml += (*it2)->writeElement();";
        code.unindent();
        code += "}";
      } else {
        code += "xml += " + type + "()->writeElement()";
      }
    }

    code += "indent( -2 );";

    code += "xml += indent() + \"</" + element->name + ">\\n\";";
  }

  code += "return xml;";

  writer.setBody( code );
  
  c.addFunction( writer );
}

void Creator::createElementParser( KODE::Class &c, Element *e )
{
  switch ( mXmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
      createElementParserDom( c, e );
      break;
    case XmlParserCustomExternal:
      createElementParserCustom( c, e );
      break;
  }
}

void Creator::createTextElementParserCustom( KODE::Class &, Element *e )
{
  KODE::Function parser( "parseElement" + upperFirst( e->name ), "TQString" );

  KODE::Code code;
  
  code += "TQString result;";
  code.newLine();

  KODE::StateMachine sm;

  KODE::Code stateCode;

  stateCode += "if ( c == '<' ) {";
  stateCode += "  state = TAG;";
  stateCode += "  tagStart = mRunning;";
  stateCode += "} else if ( c == '&' ) {";
  stateCode += "  entityStart = mRunning + 1;";
  stateCode += "} else if ( entityStart >= 0 && c == ';' ) {";
  stateCode += "  TQString entity = mBuffer.mid( entityStart, mRunning - entityStart );";
  stateCode += "  if ( entity == \"quot\" ) result += '\"';";
  stateCode += "  entityStart = -1;";
  stateCode += "} else if ( entityStart < 0 ) {";
  stateCode += "  result += c;";
  stateCode += "}";

  sm.setState( "TEXT", stateCode );

  stateCode.clear();
  stateCode += "if ( c == '/' ) {";
  stateCode += "  state = ENDTAG;";
  stateCode += "} else {";
  stateCode += "  state = STARTTAG;";
  stateCode += "}";

  sm.setState( "TAG", stateCode );

  stateCode.clear();
  stateCode += "if ( c == '>' ) {";
  stateCode += "  state = TEXT;";
  stateCode += "  result += mBuffer.mid( tagStart, mRunning - tagStart + 1 );";
  stateCode += "}";

  sm.setState( "STARTTAG", stateCode );

  stateCode.clear();
  stateCode += "if ( c == '>' ) {";
  stateCode += "  state = TEXT;";
  stateCode += "  result += mBuffer.mid( tagStart, mRunning - tagStart + 1 );";
  stateCode += "} else if ( foundText" + upperFirst( e->name ) + "() ) {";
  stateCode += "  return result;";
  stateCode += "}";

  sm.setState( "ENDTAG", stateCode );

  sm.setInitialState( "STARTTAG" );

  code.addBlock( sm.stateDefinition() );
  code.newLine();
  code += "int tagStart = -1;";
  code += "int entityStart = -1;";
  code.newLine();
  code += "while ( mRunning < mBuffer.length() ) {";
  code.indent();
  code += "TQChar c = mBuffer[ mRunning ];";
  code.addBlock( sm.transitionLogic() );
  code += "++mRunning;";
  code.unindent();
  code += "}";
  code.newLine();
  code += "return result;";

  parser.setBody( code );

  mParserClass.addFunction( parser );
}

void Creator::createElementParserCustom( KODE::Class &c, Element *e )
{
  KODE::Function parser( "parseElement" + upperFirst( e->name ),
                         c.name() + " *" );

  KODE::Code code;
  
  code += c.name() + " *result = new " + c.name() + "();";
  code.newLine();

  KODE::StateMachine sm;
  
  if ( !e->isEmpty ) {
    KODE::Code stateCode;
    stateCode += "if ( c == '<' ) state = TAG;";

    sm.setState( "WHITESPACE", stateCode );
    
    stateCode.clear();
    stateCode += "if ( c == '/' ) {";
    stateCode += "  state = ENDTAG;";
    stateCode += "} else {";
    stateCode += "  state = STARTTAG;";
    stateCode += "}";
  
    sm.setState( "TAG", stateCode );

    stateCode.clear();
    if ( e->attributes.isEmpty() ) {
      stateCode += " if ( c == '/' ) {";
      stateCode += "    return result;";
      stateCode += " }";
    }
    stateCode += "if ( c == '>' ) {";
    stateCode += "  state = WHITESPACE;";
    Element::List::ConstIterator it;
    for( it = e->elements.begin(); it != e->elements.end(); ++it ) { 
      createFoundTextFunction( (*it)->name );

      TQString eName = upperFirst( (*it)->name );
      stateCode += "} else if ( foundText" + eName + "() ) {";
      TQString line = "  result->";
      if ( (*it)->hasText ) line += "set";
      else line += "add";
      line += eName + "( parseElement" + eName + "() );";
      stateCode += line;
      stateCode += "  state = WHITESPACE;";
    }
    Reference::List::ConstIterator it3;
    for( it3 = e->references.begin(); it3 != e->references.end(); ++it3 ) { 
      createFoundTextFunction( (*it3)->name );

      TQString eName = upperFirst( (*it3)->name );
      stateCode += "} else if ( foundText" + eName + "() ) {";
      stateCode += "  result->add" + eName + "( parseElement" + eName + "() );";
      stateCode += "  state = WHITESPACE;";
    }
    stateCode += "}";

    sm.setState( "STARTTAG", stateCode );

    stateCode.clear();
    stateCode += "if ( c == '>' ) {";
    stateCode += "  state = WHITESPACE;";
    stateCode += "} else if ( foundText" + c.name() + "() ) {";
    stateCode += "  return result;";
    stateCode += "}";

    sm.setState( "ENDTAG", stateCode );

    if ( !e->attributes.isEmpty() ) {
      stateCode.clear();
      stateCode += "if ( c == '>' ) {";
      stateCode += "  state = WHITESPACE;";
      stateCode += "}";

      Attribute::List::ConstIterator it2;
      for( it2 = e->attributes.begin(); it2 != e->attributes.end(); ++it2 ) {
        bool first = it2 == e->attributes.begin();
        stateCode.addBlock( createAttributeScanner( *it2, first ) );
      }
      stateCode += "} else if ( c =='/' ) {";
      stateCode += "  return result;";
      stateCode += "}";

      sm.setState( "ATTRIBUTES", stateCode );

      sm.setInitialState( "ATTRIBUTES" );
    } else {
      sm.setInitialState( "STARTTAG" );
    }

    code.addBlock( sm.stateDefinition() );
    code.newLine();
  }
  
  if ( !e->attributes.isEmpty() ) {
    Attribute::List::ConstIterator it;
    for( it = e->attributes.begin(); it != e->attributes.end(); ++it ) {
      code += "bool found" + upperFirst( (*it)->name ) + " = false;";
    }
    code.newLine();
    code += "int attrValueStart = -1;";
    code.newLine();
  }

  code += "while ( mRunning < mBuffer.length() ) {";
  code.indent();
  code += "TQChar c = mBuffer[ mRunning ];";

  if ( e->isEmpty ) {
    code += "if ( c == '>' ) {";
    code += "  return result;";
    code += "}";

    if ( !e->attributes.isEmpty() ) {
      Attribute::List::ConstIterator it;
      for( it = e->attributes.begin(); it != e->attributes.end(); ++it ) {
        code.addBlock( createAttributeScanner( *it,
                                               it == e->attributes.begin() ) );
      }
      code += "}";
    }
  } else {
    code.addBlock( sm.transitionLogic() );
  }

  code += "++mRunning;";
  code.unindent();
  code += "}";
  code.newLine();
  code += "return result;";

  parser.setBody( code );
  
  mParserClass.addFunction( parser );
}

KODE::Code Creator::createAttributeScanner( Attribute *a, bool firstAttribute )
{
  KODE::Code code;

  TQString aName = upperFirst( a->name );

  createFoundTextFunction( a->name );

  TQString line;
  if ( !firstAttribute ) line = "} else ";
  line += "if ( foundText" + aName + "() ) {";
  code += line;
  code += "  found" + aName + "= true;";
  code += "} else if ( found" + aName + " && c == '\"' ) {";
  code += "  if ( attrValueStart < 0 ) {";
  code += "    attrValueStart = mRunning + 1;";
  code += "  } else {";
  code += "    result->set" + aName + "( mBuffer.mid( attrValueStart,";
  code += "                                  mRunning - attrValueStart ) );";
  code += "    attrValueStart = -1;";
  code += "    found" + aName + " = false;";
  code += "  }";

  return code;  
}

void Creator::createElementParserDom( KODE::Class &c, Element *e )
{
  TQString functionName;
  if ( externalParser() ) functionName = "parseElement" + c.name();
  else functionName = "parseElement";

  KODE::Function parser( functionName, c.name() + " *" );
  parser.setStatic( true );
  parser.setDocs( "Parse XML object from DOM element." );

  parser.addArgument( "const TQDomElement &element" );

  KODE::Code code;

  code += "if ( element.tagName() != \"" + e->name + "\" ) {";
  code.indent();
  code += "kdError() << \"Expected '" + e->name + "', got '\" << " +
          "element.tagName() << \"'.\" << endl;";
  code += "return 0;";
  code.unindent();
  code += "}";
  code.newLine();

  code += c.name() + " *result = new " + c.name() + "();";
  code.newLine();

  code += "TQDomNode n;";
  code += "for( n = element.firstChild(); !n.isNull();"
              " n = n.nextSibling() ) {";
  code.indent();
  code += "TQDomElement e = n.toElement();";

  TQValueList<Element *>::ConstIterator it;
  for( it = e->elements.begin(); it != e->elements.end(); ++it ) {
    TQString condition;
    if ( it != e->elements.begin() ) condition = "else ";
    condition += "if";

    code += condition + " ( e.tagName() == \"" + (*it)->name + "\" ) {";
    code.indent();

    TQString className = upperFirst( (*it)->name );

    if ( (*it)->hasText ) {
      code += "result->set" + className + "( e.text() );"; 
    } else {
      TQString line = className + " *o = ";
      if ( externalParser() ) {
        line += "parseElement" + className;
      } else {
        line += className + "::parseElement";
      }
      line += "( e );";
      code += line;
      
      code += "if ( o ) result->add" + className + "( o );";
    }

    code.unindent();
    code += "}";
  }

  code.newLine();

  TQValueList<Reference *>::ConstIterator it3;
  for( it3 = e->references.begin(); it3 != e->references.end(); ++it3 ) {
    TQString condition;
    if ( it3 != e->references.begin() ) condition = "else ";
    condition += "if";

    code += condition + " ( e.tagName() == \"" + (*it3)->name + "\" ) {";
    code.indent();

    TQString className = upperFirst( (*it3)->name );

    TQString line = className + " *o = ";
    if ( externalParser() ) {
      line += "parseElement" + className;
    } else {
      line += className + "::parseElement";
    }
    line += "( e );";
    code += line;

    code += "if ( o ) result->add" + className + "( o );";

    code.unindent();
    code += "}";
  }

  code.unindent();
  code += "}";
  code.newLine();

  TQValueList<Attribute *>::ConstIterator it2;
  for( it2 = e->attributes.begin(); it2 != e->attributes.end(); ++it2 ) {
    code += "result->set" + upperFirst( (*it2)->name ) +
            "( element.attribute( \"" + (*it2)->name + "\" ) );";
  }
  code.newLine();

  code += "return result;";

  parser.setBody( code );

  if ( externalParser() ) {
    mParserClass.addFunction( parser );
  } else {
    c.addFunction( parser );
  }
}

void Creator::registerListTypedef( const TQString &type )
{
  if ( !mListTypedefs.contains( type ) ) mListTypedefs.append( type );
}

void Creator::createListTypedefs()
{
  TQStringList::ConstIterator it;
  for( it = mListTypedefs.begin(); it != mListTypedefs.end(); ++it ) {
    KODE::Class c = mFile.findClass( *it );
    if ( !c.isValid() ) continue;
    c.addTypedef( KODE::Typedef( "TQValueList<" + *it + " *>", "List" ) );
    mFile.insertClass( c );
  }
}

void Creator::createIndenter( KODE::File &file )
{
  KODE::Function indenter( "indent", "TQString" );
  indenter.addArgument( "int n = 0" );
  
  KODE::Code code;

  code += "static int i = 0;";
  code += "i += n;";
  code += "TQString space;";
  code += "return space.fill( ' ', i );"; 

  indenter.setBody( code );

  file.addFileFunction( indenter );
}

void Creator::createFileWriter( Element *element, const TQString &dtd )
{
  TQString className = upperFirst( element->name );

  KODE::Class c = mFile.findClass( className );

  c.addInclude( "kdebug.h" );
  c.addInclude( "tqtextstream.h" );
  c.addInclude( "tqfile.h" );

  if ( !externalWriter() ) {
    createIndenter( mFile );
  }

  KODE::Function writer( "writeFile", "bool" );

  writer.addArgument( "const TQString &filename" );

  c.addInclude( "tqfile.h" );

  KODE::Code code;

  code += "TQFile file( filename );";
  code += "if ( !file.open( IO_WriteOnly ) ) {";
  code += "  kdError() << \"Unable to open file '\" << filename << \"'\" << endl;";
  code += "  return false;";
  code += "}";
  code += "";
  code += "TQTextStream ts( &file );";

  code += "ts << \"<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\"?>\\n\";";
  code += "ts << \"<!DOCTYPE features SYSTEM \\\"" + dtd + "\\\">\\n\";";

  code += "ts << writeElement();";
  code += "file.close();";
  code += "";
  code += "return true;";

  writer.setBody( code );

  c.addFunction( writer );

  mFile.insertClass( c );
}

void Creator::createFileParser( Element *element )
{
  switch ( mXmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
      createFileParserDom( element );
      break;
    case XmlParserCustomExternal:
      createFileParserCustom( element );
      break;
  }
}

void Creator::createFileParserCustom( Element *element )
{
  kdDebug() << "Creator::createFileParserCustom()" << endl;

  TQString className = upperFirst( element->name );

  KODE::Function parser( "parseFile", className + " *" );

  parser.addArgument( "const TQString &filename" );

  mParserClass.addInclude( "tqfile.h" );
  mParserClass.addInclude( "kdebug.h" );

  mParserClass.addMemberVariable( KODE::MemberVariable( "mBuffer",
                                                        "TQString" ) );
  mParserClass.addMemberVariable( KODE::MemberVariable( "mRunning",
                                                        "unsigned int" ) );

  KODE::Code code;

  code += "TQFile file( filename );";
  code += "if ( !file.open( IO_ReadOnly ) ) {";
  code += "  kdError() << \"Unable to open file '\" << filename << \"'\" << endl;";
  code += "  return 0;";
  code += "}";
  code += "";
  code += "TQTextStream ts( &file );";
  code += "mBuffer = ts.read();";
  code += "";
  code += "mRunning = 0;";
  code.newLine();

  KODE::StateMachine sm;
  
  KODE::Code stateCode;
  
  stateCode += "if ( c == '<' ) state = TAG;";

  sm.setState( "WHITESPACE", stateCode );
  
  stateCode.clear();
  
  stateCode += "if ( c == '>' ) {";
  stateCode += "  state = WHITESPACE;";
  stateCode += "} else if ( foundText" + className + "() ) {";
  stateCode += "  " + element->name + " = parseElement" + className + "();";
  stateCode += "  state = WHITESPACE;";
  stateCode += "}";

  createFoundTextFunction( element->name );

  sm.setState( "TAG", stateCode );

  code.addBlock( sm.stateDefinition() );
  code.newLine();
  
  code += className + " *" + element->name + " = 0;";
  code.newLine();

  code += "while ( mRunning < mBuffer.length() ) {";
  code.indent();
  code += "TQChar c = mBuffer[ mRunning ];";
  code.addBlock( sm.transitionLogic() );
  code += "++mRunning;";
  code.unindent();
  code += "}";
  code.newLine();

  code += "return " + element->name + ";";

  parser.setBody( code );
  
  mParserClass.addFunction( parser );
}

void Creator::createFoundTextFunction( const TQString &text )
{
  TQString functionName = "foundText" + upperFirst( text );

  if ( mParserClass.hasFunction( functionName ) ) return;

  KODE::Function f( functionName, "bool" );

  KODE::Code code;

  code += "if ( mBuffer[ mRunning ] != '" + text.right( 1 ) + "' ) return false;";
  code += "";
  code += "return mBuffer.mid( mRunning - " +
          TQString::number( text.length() - 1 ) + ", " +
          TQString::number( text.length() ) + " ) == \"" + text + "\";";

  f.setBody( code );
  
  mParserClass.addFunction( f );
}

void Creator::createFileParserDom( Element *element )
{
  kdDebug() << "Creator::createFileParserDom()" << endl;

  TQString className = upperFirst( element->name );

  KODE::Class c;

  if ( externalParser() ) {
    c = mParserClass;
  } else {
    c = mFile.findClass( className );
  }

  KODE::Function parser( "parseFile", className + " *" );
  parser.setStatic( true );

  parser.addArgument( "const TQString &filename" );

  c.addInclude( "tqfile.h" );
  c.addInclude( "tqdom.h" );
  c.addInclude( "kdebug.h" );

  KODE::Code code;

  code += "TQFile file( filename );";
  code += "if ( !file.open( IO_ReadOnly ) ) {";
  code += "  kdError() << \"Unable to open file '\" << filename << \"'\" << endl;";
  code += "  return 0;";
  code += "}";
  code += "";
  code += "TQString errorMsg;";
  code += "int errorLine, errorCol;";
  code += "TQDomDocument doc;";
  code += "if ( !doc.setContent( &file, false, &errorMsg, &errorLine, &errorCol ) ) {";
  code += "  kdError() << errorMsg << \" at \" << errorLine << \",\" << errorCol << endl;";
  code += "  return 0;";
  code += "}";
  code += "";
  code += "kdDebug() << \"CONTENT:\" << doc.toString() << endl;";

  code += "";
  
  TQString line = className + " *c = parseElement";
  if ( externalParser() ) line += className;
  line += "( doc.documentElement() );";
  code += line;
  
  code += "return c;";

  parser.setBody( code );

  c.addFunction( parser );

  if ( externalParser() ) {
    mParserClass = c;
  } else {
    mFile.insertClass( c );
  }
}

void Creator::printFiles( KODE::Printer &printer )
{
  if ( externalParser() ) {
    KODE::File parserFile( file() );
    parserFile.setFilename( file().filename() + "_parser" );

    parserFile.clearCode();
    
    mParserClass.addHeaderInclude( file().filename() + ".h" );
    parserFile.insertClass( mParserClass );
    
    kdDebug() << "Print external parser." << endl;
    printer.printHeader( parserFile );
    printer.printImplementation( parserFile );
  }

  kdDebug() << "Print header" << endl;
  printer.printHeader( file() );

  kdDebug() << "Print implementation" << endl;
  printer.printImplementation( file() );

}

bool Creator::externalParser() const
{
  return mXmlParserType == XmlParserDomExternal ||
         mXmlParserType == XmlParserCustomExternal;
}

bool Creator::externalWriter() const
{
  return mXmlWriterType == XmlWriterCustomExternal;
}

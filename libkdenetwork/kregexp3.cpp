// kregexp3.cpp - A KRegExp replacement based on QRegExp from Qt3.x
// Copyright: (c) 2001 Marc Mutz <mutz@kde.org>
// Licence: GPL

#include "kregexp3.h"

QString KRegExp3::replace( const QString & str,
			   const QString & replacementStr,
			   int start, bool global )
{
  int oldpos, pos;

  //-------- parsing the replacementStr into
  //-------- literal parts and backreferences:
  QStringList     literalStrs;
  QValueList<int> backRefs;

  // Due to LTS: The regexp in unquoted form and with spaces:
  // \\ (\d) | \$ (\d) | \$ \{ (\d+) \}
  QRegExp rx( "\\\\(\\d)|\\$(\\d)|\\$\\{(\\d+)\\}" );
  QRegExp bbrx("\\\\");
  QRegExp brx("\\");

  //  qDebug( "Analyzing replacementStr: \"" + replacementStr + "\"");

  oldpos = 0;
  pos = 0;
  while ( true ) {
    pos = rx.search( replacementStr, pos );
    
    //qDebug( QString("  Found match at pos %1").arg(pos) );

    if ( pos < 0 ) {
      literalStrs << replacementStr.mid( oldpos )
	.replace( bbrx, "\\" )
	.replace( brx, "" );
      //qDebug( QString("  No more matches. Last literal is \"") + literalStrs.last() + QString("\"") );
      break;
    } else {
      literalStrs << replacementStr.mid( oldpos, pos-oldpos )
	.replace( bbrx, "\\" )
	.replace( brx, "" );
      //qDebug( QString("  Inserting \"") + literalStrs.last() + QString("\" as literal.") );
      //qDebug( "    Searching for corresponding digit(s):" );
      for ( int i = 1 ; i < 4 ; i++ )
	if ( !rx.cap(i).isEmpty() ) {
	  backRefs << rx.cap(i).toInt();
	  //  qDebug( QString("      Found %1 at position %2 in the capturedTexts.")
	  //  .arg(backRefs.last()).arg(i) );
	  break;
	}
      pos += rx.matchedLength();
      //      qDebug( QString("  Setting new pos to %1.").arg(pos) );
      oldpos = pos;
    }
  }

  //  qDebug( "Finished the analysis of replacementStr!" );
  Q_ASSERT( literalStrs.count() == backRefs.count() + 1 );

  //-------- actual construction of the
  //-------- resulting QString
  QString result = "";
  oldpos = 0;
  pos = start;

  QStringList::Iterator sIt;
  QValueList<int>::Iterator iIt;

  if ( start < 0 )
    start += str.length();

  //qDebug( "Constructing the resultant string starts now:" );
  
  while ( true ) {
    pos = search( str, pos );

    //qDebug( QString("  Found match at pos %1").arg(pos) );

    if ( pos < 0 ) {
      result += str.mid( oldpos );
      //qDebug( "   No more matches. Adding trailing part from str:" );
      //qDebug( "    result == \"" + result + "\"" );
      break;
    } else {
      result += str.mid( oldpos, pos-oldpos );
      //qDebug( "   Adding unchanged part from str:" );
      //qDebug( "    result == \"" + result + "\"" );
      for ( sIt = literalStrs.begin(), iIt = backRefs.begin() ;
            iIt != backRefs.end() ; ++sIt, ++iIt ) {
	result += (*sIt);
	//qDebug( "   Adding literal replacement part:" );
	//qDebug( "    result == \"" + result + "\"" );
	result += cap( (*iIt) );
	//qDebug( "   Adding captured string:" );
	//qDebug( "    result == \"" + result + "\"" );
      }
      result += (*sIt);
      //qDebug( "   Adding literal replacement part:" );
      //qDebug( "    result == \"" + result + "\"" );
    }
    pos += matchedLength();
    //qDebug( QString("  Setting new pos to %1.").arg(pos) );
    oldpos = pos;

    if ( !global ) {
      // only replace the first occurrence, so stop here:
      result += str.mid( oldpos );
      break;
    }
  }

  return result;
}

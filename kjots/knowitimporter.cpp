/*
 * This file is part of KJots
 *
 * Copyright 2008 Stephen Kelly <steveire@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */


#include "knowitimporter.h"


#include <QFile>
#include <QTextStream>


#include <KUrl>
#include <QTemporaryFile>
#include <KLocalizedString>

#include <qdebug.h>
#include <QStandardPaths>

KnowItImporter::KnowItImporter()
{
}

void KnowItImporter::importFromUrl( const KUrl& url )
{
// TODO PORT
#if 0
  KJotsBook *book = new KJotsBook();
  buildNoteTree(url);

//   foreach ()
//   qDebug();
  buildDomDocument();


  QTemporaryFile file;
//code was   file.setPrefix( QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "kjots/" ) ;
//Add to constructor and adapt if necessay:  QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "kjots/" QLatin1String("/myapp_XXXXXX.txt") 
//code was   file.setSuffix( ".book" );
//Add to constructor and adapt if necessay: QDir::tempPath() + QLatin1String("/myapp_XXXXXX") + QLatin1String( ".book" ) 
  file.setAutoRemove( false );

  if ( file.open() ) {
    file.write( "<?xml version='1.0' encoding='UTF-8' ?>\n<!DOCTYPE KJots>\n<KJots>\n" );
    file.write( m_domDoc.toByteArray() );
    file.write( "</KJots>\n" );
    qDebug() << file.fileName();
    QString newFileName = file.fileName();
    file.close();
    book->openBook( newFileName );
  }


  return book;
#endif
}

QDomElement KnowItImporter::addNote( const KnowItNote& note)
{
  QDomElement newElement;
  int childNotesCount = m_childNotes[ note.id ].size();
//   int childNotesCount = note.childNotes.size();
  qDebug() << note.title << childNotesCount;
  if (childNotesCount > 0)
  {
    newElement = m_domDoc.createElement(QLatin1String("KJotsBook"));

  } else {
    newElement = m_domDoc.createElement(QLatin1String("KJotsPage"));
  }

  QDomElement titleTag = m_domDoc.createElement( QLatin1String("Title") );
  titleTag.appendChild( m_domDoc.createTextNode( note.title ) );
  newElement.appendChild( titleTag );
  QDomElement idTag = m_domDoc.createElement( QLatin1String("ID") );
  idTag.appendChild( m_domDoc.createTextNode( QLatin1String("0") ) );   // Gets a valid id later.
  newElement.appendChild( idTag );

  if (childNotesCount > 0)
  {
    QDomElement openTag = m_domDoc.createElement( QLatin1String("Open") );
    openTag.appendChild( m_domDoc.createTextNode( QLatin1String("1") ) );
    newElement.appendChild( openTag );

    QDomElement titlePage = m_domDoc.createElement(QLatin1String("KJotsPage"));
    QDomElement titlePageTitleTag = m_domDoc.createElement( QLatin1String("Title") );
    titlePageTitleTag.appendChild( m_domDoc.createTextNode( note.title ) );
    titlePage.appendChild( titlePageTitleTag );
    QDomElement titlePageIdTag = m_domDoc.createElement( QLatin1String("ID" ));
    titlePageIdTag.appendChild( m_domDoc.createTextNode( QLatin1String("0") ) );   // Gets a valid id later.
    titlePage.appendChild( titlePageIdTag );
    QDomElement titlePageTextTag = m_domDoc.createElement( QLatin1String("Text" ));
    titlePageTextTag.appendChild( m_domDoc.createCDATASection( note.content ) );
    titlePage.appendChild( titlePageTextTag );
    newElement.appendChild( titlePage );

    foreach (int id, m_childNotes[ note.id ] )
    {
      QDomElement e = addNote( m_noteHash.value(id) );
      newElement.appendChild(e);
    }
  } else {
      QString contents = note.content;
      if ( note.links.size() > 0 ) {
        if ( contents.endsWith( QLatin1String("</body></html>") ) ) {
          contents.chop( 14 );
        }
        contents.append( QLatin1String("<br /><br /><p><b>Links:</b></p>\n<ul>\n") );
        for ( int i = 0; i < note.links.size(); ++i ) {
          qDebug() << "link" << note.links[i].first << note.links[i].second;
          contents.append( QString::fromLatin1( "<li><a href=\"%1\">%2</a></li>\n" )
              .arg( note.links[i].first )
              .arg( note.links[i].second ) );
        }
        contents.append( QLatin1String("</ul></body></html>") );
      }


    QDomElement textTag = m_domDoc.createElement( QLatin1String("Text") );
    textTag.appendChild( m_domDoc.createCDATASection( contents ) );
    newElement.appendChild( textTag );
  }

  return newElement;


}

void KnowItImporter::buildDomDocument()
{
  QDomElement parent = m_domDoc.createElement( QLatin1String("KJotsBook") );
  QDomElement titleTag = m_domDoc.createElement( QLatin1String("Title") );
  titleTag.appendChild( m_domDoc.createTextNode( i18nc("Name for the top level book created to hold the imported data.", "KNowIt Import") ) );
  parent.appendChild( titleTag );
  QDomElement idTag = m_domDoc.createElement( QLatin1String("ID") );
  idTag.appendChild( m_domDoc.createTextNode( QLatin1String("0") ) );   // Gets a valid id later.
  parent.appendChild( idTag );
  QDomElement openTag = m_domDoc.createElement( QLatin1String("Open") );
  openTag.appendChild( m_domDoc.createTextNode( QLatin1String("1") ) );
  parent.appendChild( openTag );
  m_domDoc.appendChild( parent );

  foreach (const KnowItNote &n, m_notes)
  {
    QDomElement e = addNote( n );
    parent.appendChild(e);
    qDebug() << n.title;
  }
  qDebug() << m_domDoc.toString();
}

void KnowItImporter::buildNoteTree( const KUrl& url )
{

  QFile knowItFile( url.toLocalFile() );
  if ( knowItFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
    QSet<QByteArray> entryHeaders;
    entryHeaders << "\\NewEntry" << "\\CurrentEntry";

    int id = 1;

    QTextStream in( &knowItFile );
    while ( !in.atEnd() ) {
      QString line = in.readLine();

      qDebug() << "got line: " << line;

      if ( line.trimmed().isEmpty() ) {
        continue;
      }

      foreach( const QByteArray &header, entryHeaders ) {
        if ( line.startsWith( QLatin1String(header) ) ) {
          qDebug() << "init" << line << header;
          line = line.right( line.size() - header.size() ).trimmed();
          qDebug() << "header tag removed: " << line;

          QStringList list = line.split( QLatin1Char(' ') );
          int startOfTitle = line.indexOf( QLatin1Char(' ') );
          bool ok = false;

          qDebug() << "depth" << list.at( 0 ).trimmed();

          int depth = list.at( 0 ).trimmed().toInt( &ok );
          qDebug() << ok << "valid depth";
          if ( ok ) {
            QString title = line.right( line.size() - startOfTitle ).trimmed();
            KnowItNote n;
            n.title = title;
            n.depth = depth;
            n.id = id;
            if (depth == 0)
            {
              n.parent = 0;
            } else {
              n.parent = m_lastNoteAtLevel[depth - 1].id;
            }

            QString contentLine = in.readLine();
            //QList< QPair <QString, QString> > links;
            QString contents;
            QString url;
            QString target;
            while (( !in.atEnd() ) && ( !contentLine.trimmed().isEmpty() ) ) {
              qDebug() << contentLine;
              if ( contentLine.startsWith( QLatin1String("\\Link") ) ) {
                url = contentLine.right( contentLine.size() - 5 ).trimmed();
                contentLine = in.readLine();
                continue;
              }
              if ( contentLine.startsWith( QLatin1String("\\Descr") ) ) {
                target = contentLine.right( contentLine.size() - 6 ).trimmed();
                contentLine = in.readLine();
                continue;
              }
              if ( !url.isEmpty() && !target.isEmpty() ) {
                QPair< QString, QString > link;
                link.first = url;
                link.second = target;
                n.links << link;
                url.clear();
                target.clear();
              }
              contents.append( contentLine );
              contentLine = in.readLine();
            }


            n.content = contents;

            m_noteHash.insert(id, n);
            m_childNotes[n.parent].append(id);
            id++;

            if ( m_lastNoteAtLevel.size() == depth )
            {
              m_lastNoteAtLevel.append(n);
            } else {
              m_lastNoteAtLevel[depth] = n;
            }

            if (depth == 0)
            {
              m_notes.append(n);
            }
          }
          break; // If found first header, don't check for second one.
        }
      } // Foreach header.
    } // At end of stream
  } // File open.
}



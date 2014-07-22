/*
    This file is part of kdepim.

    Copyright (c) 2004 Till Adam <adam@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <messageviewer/interfaces/bodypartformatter.h>
#include <messageviewer/interfaces/bodypart.h>
#include <messageviewer/interfaces/bodyparturlhandler.h>
#include <messageviewer/htmlwriter/webkitparthtmlwriter.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <qdebug.h>
#include <kmessagebox.h>

#include <kapplication.h>


#include <QUrl>
#include <QFile>
#include <QDir>
#include <QTextDocument>
#include <KLocale>

namespace {

  // TODO: Show filename header to make it possible to save the patch.
  // FIXME: The box should only be as wide as necessary.

  class Formatter : public MessageViewer::Interface::BodyPartFormatter {
  public:
    Result format( MessageViewer::Interface::BodyPart *bodyPart, MessageViewer::HtmlWriter *writer ) const {

      if ( !writer ) return Ok;

      if (  bodyPart->defaultDisplay() == MessageViewer::Interface::BodyPart::AsIcon )
        return AsIcon;

      const QString diff = bodyPart->asText();
      if ( diff.isEmpty() ) return AsIcon;

      static const QLatin1String addedLineStyle( "style=\"" "color: green;\"" );
      static const QLatin1String fileAddStyle( "style=\"font-weight: bold; " "color: green; \"" );

      static const QLatin1String removedLineStyle( "style=\"" "color: red;\"" );
      static const QLatin1String fileRemoveStyle( "style=\"font-weight: bold; "
                               "color: red ;\"" );

      static const QLatin1String tableStyle(
        "style=\""
        "text-align: left; "
        "border: solid black 1px; "
        "padding: 0.5em; "
        "margin: 0em;\"");

      static const QLatin1String sepStyle( "style=\"color: black; font-weight: bold;\"" );
      static const QLatin1String chunkStyle( "style=\"color: blue;\"" );

      QString html = QLatin1String("<br><div align=\"center\">");
      html += QLatin1String("<pre ") + tableStyle + QLatin1Char('>');

      const QStringList lines = diff.split( QLatin1Char('\n') );
      QStringList::ConstIterator end( lines.end() );
      for ( QStringList::ConstIterator it = lines.begin(); it != end; ++it ) {
        const QString line( Qt::escape( *it ) );
        QString style;
        if ( !line.isEmpty() ) {
          if ( line.startsWith( QLatin1String("+++") ) ) {
            style = fileAddStyle;
          } else if ( line.startsWith( QLatin1String("---") ) ) {
            style = fileRemoveStyle;
          } else if ( line.startsWith( QLatin1Char('+') ) || line.startsWith( QLatin1Char('>') ) ) {
            style = addedLineStyle;
          } else if ( line.startsWith( QLatin1Char('-') ) || line.startsWith( QLatin1Char('<') ) ) {
            style = removedLineStyle;
          } else if ( line.startsWith( QLatin1String("==") ) ) {
            style = sepStyle;
          } else if ( line.startsWith( QLatin1String("@@") ) ) {
            style = chunkStyle;
          }
        }
        html += QLatin1String("<span ") + style + QLatin1Char('>') + line + QLatin1String("</span><br/>");
      }

      html += QLatin1String("</pre></div>");
      //qDebug( "%s", html.toLatin1() );
      writer->queue( html );

      return Ok;
    }

    // unhide the overload with three arguments
    using MessageViewer::Interface::BodyPartFormatter::format;
  };

  class Plugin : public MessageViewer::Interface::BodyPartFormatterPlugin {
  public:
    const MessageViewer::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const {
      return idx == 0 ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const {
      return idx == 0 ? "text" : 0 ;
    }
    const char * subtype( int idx ) const {
      return idx == 0 ? "x-diff" : 0 ;
    }

    const MessageViewer::Interface::BodyPartURLHandler * urlHandler( int ) const { return 0; }
  };

}

extern "C"
Q_DECL_EXPORT MessageViewer::Interface::BodyPartFormatterPlugin *
messageviewer_bodypartformatter_text_xdiff_create_bodypart_formatter_plugin() {
  //QT5 KLocale::global()->insertCatalog( QLatin1String("messageviewer_text_xdiff_plugin") );
  return new Plugin();
}

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <interfaces/bodypartformatter.h>
#include <interfaces/bodypart.h>
#include <interfaces/bodyparturlhandler.h>
#include <khtmlparthtmlwriter.h>

#include <kmail/callback.h>
#include <kmail/kmmessage.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include <qurl.h>
#include <qfile.h>
#include <qdir.h>
#include <qstylesheet.h>

namespace {

  // TODO: Show filename header to make it possible to save the patch.
  // FIXME: The box should only be as wide as necessary.

  class Formatter : public KMail::Interface::BodyPartFormatter {
  public:
    Result format( KMail::Interface::BodyPart *bodyPart, KMail::HtmlWriter *writer ) const {

      if ( !writer ) return Ok;

      if (  bodyPart->defaultDisplay() == KMail::Interface::BodyPart::AsIcon ) 
        return AsIcon;

      const QString diff = bodyPart->asText();
      if ( diff.isEmpty() ) return AsIcon;



      QString addedLineStyle = QString::fromLatin1(
        "style=\""
        "color: green;\"");
      QString fileAddStyle( "style=\"font-weight: bold; "
                            "color: green; \"" );

      QString removedLineStyle = QString::fromLatin1(
        "style=\""
        "color: red;\"");
      QString fileRemoveStyle( "style=\"font-weight: bold; "
                               "color: red ;\"" );

      QString tableStyle = QString::fromLatin1(
        "style=\""
        "border: solid black 1px; "
        "padding: 0.5em; "
        "margin: 0em;\"");

      QString sepStyle( "style=\"color: black; font-weight: bold;\"" );
      QString chunkStyle( "style=\"color: blue;\"" );

      QString html = "<br><div align=\"center\">";
      html += "<pre " + tableStyle + ">";

      QStringList lines = QStringList::split( '\n', diff, true );
      for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
        QString line( QStyleSheet::escape( *it ) );
        QString style;
        if ( line.length() > 0 ) {
          if ( line.startsWith( "+++" ) ) {
            style = fileAddStyle;
          } else if ( line.startsWith( "---" ) ) {
            style = fileRemoveStyle;
          } else if ( line.startsWith( "+" ) || line.startsWith( ">" ) ) {
            style = addedLineStyle;
          } else if ( line.startsWith( "-" ) || line.startsWith( "<" ) ) {
            style = removedLineStyle;
          } else if ( line.startsWith( "==") ) {
            style = sepStyle;
          } else if ( line.startsWith( "@@" ) ) {
            style = chunkStyle;
          } else {
            style = "";
          }
        }
        html += "<span " + style + ">" + line + "</span>\n";
      }

      html += "</pre></div>";
      //qDebug( "%s", html.latin1() );
      writer->queue( html );

      return Ok;
    }
  };

  class Plugin : public KMail::Interface::BodyPartFormatterPlugin {
  public:
    const KMail::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const {
      return idx == 0 ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const {
      return idx == 0 ? "text" : 0 ;
    }
    const char * subtype( int idx ) const {
      return idx == 0 ? "x-diff" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int ) const { return 0; }
  };

}

extern "C"
KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_xdiff_create_bodypart_formatter_plugin() {
  return new Plugin();
}

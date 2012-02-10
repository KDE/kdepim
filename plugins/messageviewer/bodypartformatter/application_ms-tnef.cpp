/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
#include <messageviewer/interfaces/htmlwriter.h>
#include <messageviewer/nodehelper.h>
#include <messagecore/stringutil.h>
#include <messageviewer/util.h>

#include <KPIMUtils/KFileIO>

#include <KCalCore/Event>
#include <KCalCore/Incidence>
#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>

#include <KCalUtils/IncidenceFormatter>

#include <ktnef/formatter.h>
#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefattach.h>

#include <KDebug>
#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KSystemTimeZones>
#include <KUrl>

#include <QApplication>
#include <QDir>

namespace {

  class Formatter : public MessageViewer::Interface::BodyPartFormatter {
  public:
    Result format( MessageViewer::Interface::BodyPart *bodyPart, MessageViewer::HtmlWriter *writer ) const {

      if ( !writer ) return Ok;

      const QString dir = QApplication::isRightToLeft() ? "rtl" : "ltr" ;
      QString htmlStr = "<table cellspacing=\"1\" class=\"textAtm\">";
      QString startRow = "<tr class=\"textAtmH\"><td dir=\"" + dir + "\">";
      QString endRow = "</td></tr>";

      const QString fileName = bodyPart->nodeHelper()->writeNodeToTempFile( bodyPart->content() );
      KTnef::KTNEFParser parser;
      if ( !parser.openFile( fileName ) || !parser.message()) {
        kDebug() << "Could not parse" << fileName;
        return Failed;
      }

      // Look for an invitation
      QString inviteStr;
      QByteArray buf = KPIMUtils::kFileToByteArray( fileName, false, false );
      if ( !buf.isEmpty() ) {
        KCalCore::MemoryCalendar::Ptr cl(
          new KCalCore::MemoryCalendar( KSystemTimeZones::local() ) );
        KCalUtils::InvitationFormatterHelper helper;
        const QString invite = KTnef::formatTNEFInvitation( buf, cl, &helper );
        KCalCore::ICalFormat format;
        KCalCore::Incidence::Ptr inc = format.fromString( invite );
        KCalCore::Event::Ptr event = inc.dynamicCast<KCalCore::Event>();
        if ( event && event->hasEndDate() ) {
          // no enddate => not a valid invitation
          inviteStr = KCalUtils::IncidenceFormatter::extensiveDisplayStr( cl, inc );
        }
      }

      QList<KTnef::KTNEFAttach*> tnefatts = parser.message()->attachmentList();
      if ( tnefatts.isEmpty() && inviteStr.isEmpty() ) {
        kDebug() << "No attachments or invitation found in" << fileName;

        QString label = MessageViewer::NodeHelper::fileName( bodyPart->content() );
        label = MessageCore::StringUtil::quoteHtmlChars( label, true );
        const QString comment =
          MessageCore::StringUtil::quoteHtmlChars(
            bodyPart->content()->contentDescription()->asUnicodeString(), true );

        htmlStr += startRow;
        htmlStr += label;
        if ( !comment.isEmpty() ) {
          htmlStr += "<br/>" + comment;
        }
        htmlStr += "&nbsp;&lt;" + i18nc( "TNEF attachment has no content", "empty" ) + "&gt;";
        htmlStr += endRow;
        htmlStr += "</table>";
        writer->queue( htmlStr );

        return NeedContent;
      }

      QString label = MessageViewer::NodeHelper::fileName( bodyPart->content() );
      label = MessageCore::StringUtil::quoteHtmlChars( label, true );
      const QString comment =
        MessageCore::StringUtil::quoteHtmlChars(
          bodyPart->content()->contentDescription()->asUnicodeString(), true );

      htmlStr += startRow;
      htmlStr += label;
      if ( !comment.isEmpty() ) {
        htmlStr += "<br/>" + comment;
      }
      htmlStr += endRow;
      if ( !inviteStr.isEmpty() ) {
        htmlStr += startRow;
        htmlStr += inviteStr;
        htmlStr += endRow;
      }

      if ( tnefatts.count() > 0 ) {
        htmlStr += startRow;
      }
      writer->queue( htmlStr );
      const int numberOfTnef( tnefatts.count() );
      for ( int i = 0; i < numberOfTnef; ++i ) {
        KTnef::KTNEFAttach *att = tnefatts.at( i );
        QString label = att->displayName();
        if( label.isEmpty() )
          label = att->name();
        label = MessageCore::StringUtil::quoteHtmlChars( label, true );

        const QString dir = bodyPart->nodeHelper()->createTempDir( "ktnef-" + QString::number( i ) );
        parser.extractFileTo( att->name(), dir );
        bodyPart->nodeHelper()->addTempFile( dir + QDir::separator() + att->name() );
        const QString href = "file:" + KUrl::toPercentEncoding( dir + QDir::separator() + att->name() );

        const QString iconName = MessageViewer::Util::fileNameForMimetype( att->mimeTag(),
                                                            KIconLoader::Desktop, att->name() );

        writer->queue( "<div><a href=\"" + href + "\"><img src=\"file:///" +
                              iconName + "\" border=\"0\" style=\"max-width: 100%\"/>" + label +
                              "</a></div><br/>" );
      }

      if ( tnefatts.count() > 0 ) {
        writer->queue( endRow );
      }
      writer->queue( "</table>" );

      return Ok;
    }
  };

  class Plugin : public MessageViewer::Interface::BodyPartFormatterPlugin {
  public:
    const MessageViewer::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const {
      return idx == 0 ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const {
      return idx == 0 ? "application" : 0 ;
    }
    const char * subtype( int idx ) const {
      if ( idx == 0 ) {
        return "ms-tnef";
      } else if ( idx == 1 ) {
        return "vnd.ms-tnef";
      } else {
        return 0;
      }
    }

    const MessageViewer::Interface::BodyPartURLHandler * urlHandler( int ) const { return 0; }
  };

}

extern "C"
KDE_EXPORT MessageViewer::Interface::BodyPartFormatterPlugin *
messageviewer_bodypartformatter_application_mstnef_create_bodypart_formatter_plugin() {
  KGlobal::locale()->insertCatalog( "messageviewer_application_mstnef_plugin" );
  return new Plugin();
}

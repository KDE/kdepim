/*  -*- mode: C++; c-file-style: "gnu" -*-
    text_vcard.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Till Adam <adam@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
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

#include <qurl.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

#include <kaddrbook.h>

#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"
using KMail::Interface::BodyPart;
#include "interfaces/bodyparturlhandler.h"
#include "khtmlparthtmlwriter.h"
#include <kimproxy.h>

#include <kabc/vcardconverter.h>
#include <kabc/addressee.h>
using KABC::VCardConverter;
using KABC::Addressee;

#include "addresseeview.h"
using KPIM::AddresseeView;

#include <kdepimmacros.h>

namespace {

  class Formatter : public KMail::Interface::BodyPartFormatter {
  public:
    Formatter() {
      // disabled pending resolution of how to share static objects when dlopening libraries
      //mKIMProxy = ::KIMProxy::instance( kapp->dcopClient() );
    }

    Result format( BodyPart *bodyPart, KMail::HtmlWriter *writer ) const {

       if ( !writer ) return AsIcon;

       VCardConverter vcc;
       const QString vCard = bodyPart->asText();
       if ( vCard.isEmpty() ) return AsIcon;
       Addressee::List al = vcc.parseVCards(  vCard );
       if ( al.empty() ) return AsIcon;

       writer->queue (
             "<div align=\"center\"><h2>" +
             i18n( "Attached business cards" ) +
             "</h2></div>"
                );

       QValueListIterator<KABC::Addressee> it = al.begin();
       int count = 0;
       for ( ; it != al.end(); ++it ) {
          KABC::Addressee a = (*it);
          if ( a.isEmpty() ) return AsIcon;

          QString contact = AddresseeView::vCardAsHTML( a, 0L, 0, false, true, true, true, true, true, false );
          writer->queue( contact );

          QString addToLinkText = i18n( "[Add this contact to the addressbook]" );
          QString op = QString::fromLatin1( "addToAddressBook:%1" ).arg( count );
          writer->queue(
                "<div align=\"center\"><a href=\"" +
                bodyPart->makeLink( op ) +
                "\">" +
                addToLinkText +
                "</a></div><br><br>" );
          count++;
       }

       return Ok;
    }
  private:
    //::KIMProxy *mKIMProxy;
};

  class UrlHandler : public KMail::Interface::BodyPartURLHandler {
  public:
     bool handleClick( BodyPart * bodyPart, const QString & path,
                       KMail::Callback& ) const {

       const QString vCard = bodyPart->asText();
       if ( vCard.isEmpty() ) return true;
       VCardConverter vcc;
       Addressee::List al = vcc.parseVCards(  vCard );
       int index = path.right( path.length() - path.findRev( ":" ) - 1 ).toInt();
       if ( index == -1 ) return true;
       KABC::Addressee a = al[index];
       if ( a.isEmpty() ) return true;
       KAddrBookExternal::addVCard( a, 0 );
       return true;
     }

     bool handleContextMenuRequest(  BodyPart *, const QString &, const QPoint & ) const {
       return false;
     }

     QString statusBarMessage(  BodyPart *, const QString & ) const {
       return i18n("Add this contact to the address book.");
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
      return idx == 0 ? "x-vcard" : idx == 1 ? "vcard" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int idx ) const {
       return idx == 0 ? new UrlHandler() : 0 ;
    }
  };

}

extern "C"
KDE_EXPORT KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_vcard_create_bodypart_formatter_plugin() {
  KGlobal::locale()->insertCatalogue( "kmail_text_vcard_plugin" );
  return new Plugin();
}


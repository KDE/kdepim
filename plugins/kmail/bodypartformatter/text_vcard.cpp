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

#include <qurl.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/netaccess.h>


#include <libkdepim/addresseeview.h>
#include <libkdepim/kaddrbook.h>

#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"
using KMail::Interface::BodyPart;
#include "interfaces/bodyparturlhandler.h"
#include "khtmlparthtmlwriter.h"
#include <kimproxy.h>
#include <kpopupmenu.h>

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

    Result format( BodyPart *bodyPart, KMail::HtmlWriter *writer, KMail::Callback & ) const {

       if ( !writer ) return AsIcon;

       VCardConverter vcc;
       const QString vCard = bodyPart->asText();
       if ( vCard.isEmpty() ) return AsIcon;
       Addressee::List al = vcc.parseVCardsRaw(  vCard.utf8() );
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

          QString contact = AddresseeView::vCardAsHTML( a, 0L, AddresseeView::NoLinks, false, AddresseeView::DefaultFields );
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
      Addressee::List al = vcc.parseVCardsRaw(  vCard.utf8() );
      int index = path.right( path.length() - path.findRev( ":" ) - 1 ).toInt();
      if ( index == -1 ) return true;
      KABC::Addressee a = al[index];
      if ( a.isEmpty() ) return true;
      KAddrBookExternal::addVCard( a, 0 );
      return true;
    }

    static KABC::Addressee findAddressee( BodyPart *part, const QString &path )
    {
      const QString vCard = part->asText();
      if ( !vCard.isEmpty() ) {
        VCardConverter vcc;
        Addressee::List al = vcc.parseVCardsRaw(  vCard.utf8() );
        int index = path.right( path.length() - path.findRev( ":" ) - 1 ).toInt();
        if ( index >= 0 ) {
          return al[index];
        }
      }
      return KABC::Addressee();
    }

    bool handleContextMenuRequest( KMail::Interface::BodyPart *part,
                                   const QString &path,
                                   const QPoint &point ) const
    {
      const QString vCard = part->asText();
      if ( vCard.isEmpty() ) {
        return true;
      }
      KABC::Addressee a = findAddressee( part, path );
      if ( a.isEmpty() ) {
        return true;
      }

      KPopupMenu *menu = new KPopupMenu();
      menu->insertItem( i18n( "View Business Card" ), 0 );
      menu->insertItem( i18n( "Save Business Card As..." ), 1 );

      switch( menu->exec( point, 0 ) ) {
      case 0: // open
        openVCard( a, vCard );
        break;
      case 1: // save as
        saveAsVCard( a, vCard );
        break;
      default:
        break;
      }
      return true;
    }

    QString statusBarMessage( BodyPart *part, const QString &path ) const
    {
      KABC::Addressee a = findAddressee( part, path );
      if ( a.realName().isEmpty() ) {
        return i18n( "Add this contact to the address book." );
      } else {
        return i18n( "Add \"%1\" to the address book." ).arg( a.realName() );
      }
    }

    bool openVCard( const KABC::Addressee &a, const QString &vCard ) const
    {
      Q_UNUSED( vCard );
      AddresseeView *view = new AddresseeView( 0 );
      view->setVScrollBarMode( QScrollView::Auto );
      if ( a.isEmpty() ) {
        view->setText( i18n( "Failed to parse the business card." ) );
      } else {
        view->setAddressee( a );
      }
      view->setMinimumSize( 300, 400 );
      view->show();
      return true;
    }

    bool saveAsVCard( const KABC::Addressee &a, const QString &vCard ) const
    {
      QString fileName = a.givenName() + '_' + a.familyName() + ".vcf";

      // get the saveas file name
      KURL saveAsUrl =
        KFileDialog::getSaveURL( fileName,
                                 QString::null, 0,
                                 i18n( "Save Business Card" ) );
      if ( saveAsUrl.isEmpty() ||
           ( QFileInfo( saveAsUrl.path() ).exists() &&
             ( KMessageBox::warningYesNo(
               0,
               i18n( "%1 already exists. Do you want to overwrite it?").
               arg( saveAsUrl.path() ) ) == KMessageBox::No ) ) ) {
        return false;
      }

      // put the attachment in a temporary file and save it
      KTempFile tmpFile;
      tmpFile.setAutoDelete( true );

      QByteArray data = vCard.utf8();
      tmpFile.file()->writeBlock( data.data(), data.size() );
      tmpFile.close();

      return KIO::NetAccess::upload( tmpFile.name(), saveAsUrl, 0 );
    }
  };

  class Plugin : public KMail::Interface::BodyPartFormatterPlugin {
  public:
    const KMail::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const {
      return validIndex( idx ) ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const {
      return validIndex( idx ) ? "text" : 0 ;
    }
    const char * subtype( int idx ) const {
      return idx == 0 ? "x-vcard" : idx == 1 ? "vcard" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int idx ) const {
       return validIndex( idx ) ? new UrlHandler() : 0 ;
    }
  private:
    bool validIndex( int idx ) const {
      return ( idx >= 0 && idx <= 1 );
    }
  };

}

extern "C"
KDE_EXPORT KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_vcard_create_bodypart_formatter_plugin() {
  KGlobal::locale()->insertCatalogue( "kmail_text_vcard_plugin" );
  return new Plugin();
}


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

#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"
#include "khtmlparthtmlwriter.h"

#include <kabc/vcardconverter.h>
#include <kabc/addressee.h>
using KABC::VCardConverter;
using KABC::Addressee;

namespace {

  class Formatter : public KMail::Interface::BodyPartFormatter {
  public:
    Result format( KMail::Interface::BodyPart *bodyPart, KMail::HtmlWriter *writer ) const { 

       VCardConverter vcc;
       const QString vCard = bodyPart->asText();
       if ( vCard.isEmpty() ) return AsIcon;
       Addressee::List al = vcc.parseVCards(  vCard );
       if ( al.empty() ) return AsIcon;
       QValueListIterator<KABC::Addressee> it = al.begin();


       QString header = QString::fromLatin1(
             "<div align=\"center\">"
             "<table width=\"80%\" style=\"border: #000000 thin dashed 1px;\"  >"
             );

       writer->queue( header );
       for ( ; it != al.end(); ++it ) {
          KABC::Addressee a = (*it);
          if ( a.isEmpty() ) return AsIcon;

          QString dynamicPart;
          QString name = ( a.formattedName().isEmpty() ?
                a.assembledName() : a.formattedName() );

          QString rowFmtStr = QString::fromLatin1(
                "<tr><td align=\"right\" width=\"30%\"><b>%1</b></td>"
                "<td align=\"left\" width=\"70%\">%2</td></tr>\n"
                );

          QDate date = a.birthday().date();

          if ( date.isValid() )
             dynamicPart += rowFmtStr
                .arg( KABC::Addressee::birthdayLabel() )
                .arg( KGlobal::locale()->formatDate( date, true ) );

          KABC::PhoneNumber::List phones = a.phoneNumbers();
          KABC::PhoneNumber::List::ConstIterator phoneIt;
          for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
             QString number = (*phoneIt).number();

             QString url;
             if ( (*phoneIt).type() & KABC::PhoneNumber::Fax )
                url = QString::fromLatin1("fax:") + number;
             else
                url = QString::fromLatin1("phone:") + number;

             dynamicPart += rowFmtStr
                .arg( KABC::PhoneNumber::typeLabel( (*phoneIt).type() ).replace( " ", "&nbsp;" ) )
                .arg( QString::fromLatin1("<a href=\"%1\">%2</a>").arg(url).arg(number) );
          }

          QStringList emails = a.emails();
          QStringList::ConstIterator emailIt;
          QString type = i18n( "Email" );
          for ( emailIt = emails.begin(); emailIt != emails.end(); ++emailIt ) {
             QString fullEmail = a.fullEmail( *emailIt );
             QUrl::encode( fullEmail );
             dynamicPart += rowFmtStr
                .arg( type )
                .arg( QString::fromLatin1( "<a href=\"mailto:%1\">%2</a>" )
                      .arg( fullEmail, *emailIt ) );
             type = i18n( "Other" );
          }

          if ( !a.url().url().isEmpty() ) {
             dynamicPart += rowFmtStr
                .arg( i18n( "Homepage" ) )
                .arg( KStringHandler::tagURLs( a.url().url() ) );
          }

          KABC::Address::List addresses = a.addresses();
          KABC::Address::List::ConstIterator addrIt;
          for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
             if ( (*addrIt).label().isEmpty() ) {
                QString formattedAddress;

                formattedAddress = (*addrIt).formattedAddress().stripWhiteSpace();
#if 0
                if ( !(*addrIt).street().isEmpty() )
                   formattedAddress += (*addrIt).street() + "\n";

                if ( !(*addrIt).postOfficeBox().isEmpty() )
                   formattedAddress += (*addrIt).postOfficeBox() + "\n";

                formattedAddress += (*addrIt).locality() + QString::fromLatin1(" ") + (*addrIt).region();

                if ( !(*addrIt).postalCode().isEmpty() )
                   formattedAddress += QString::fromLatin1(", ") + (*addrIt).postalCode();

                formattedAddress += "\n";

                if ( !(*addrIt).country().isEmpty() )
                   formattedAddress += (*addrIt).country() + "\n";

                formattedAddress += (*addrIt).extended();
#endif

                formattedAddress = formattedAddress.replace( '\n', "<br>" );

                QString link = "<a href=\"addr:" + (*addrIt).id() + "\">" +
                   formattedAddress + "</a>";

                dynamicPart += rowFmtStr
                   .arg( KABC::Address::typeLabel( (*addrIt).type() ) )
                   .arg( link );
             } else {
                QString link = "<a href=\"addr:" + (*addrIt).id() + "\">" +
                   (*addrIt).label().replace( '\n', "<br>" ) + "</a>";

                dynamicPart += rowFmtStr
                   .arg( KABC::Address::typeLabel( (*addrIt).type() ) )
                   .arg( link );
             }
          }

          QString notes;
          if ( !a.note().isEmpty() ) {
             notes = QString::fromLatin1(
                   "<tr>"
                   "<td align=\"right\" valign=\"top\" width=\"30%\"><b>%1:</b></td>"  // note label
                   "<td align=\"left\" valign=\"top\">%2</td>"  // note
                   "</tr>" ).arg( i18n( "Notes" ) ).arg( a.note().replace( '\n', "<br>" ) );
          }

          QString role, organization;
          role = a.role();
          organization = a.organization();

          // when only an organization is set we use it as name
          if ( !a.organization().isEmpty() && name.isEmpty() ||
                a.formattedName() == a.organization() ) {
             name = a.organization();
             organization = QString::null;
          }

          QString personStr = QString::fromLatin1(
                "<tr>"
                "<td align=\"right\" valign=\"top\" width=\"30%\" rowspan=\"3\">"
                "<img src=\"myimage\" width=\"50\" height=\"70\">"
                "</td>"
                "<td align=\"left\" width=\"70%\"><font size=\"+2\"><b>%1</b></font></td>"  // name
                "</tr>"
                "<tr>"
                "<td align=\"left\" width=\"70%\">%2</td>"  // role
                "</tr>"
                "<tr>"
                "<td align=\"left\" width=\"70%\">%3</td>"  // organization
                "</tr>"
                "</tr>"
                "<tr><td colspan=\"2\">&nbsp;</td></tr>"
                "%4"  // dynamic part
                "%5"  // notes
                )
             .arg( name )
             .arg( role )
             .arg( organization )
             .arg( dynamicPart, notes );


          writer->queue( personStr );
          KABC::Picture picture = a.photo();
          if ( picture.isIntern() && !picture.data().isNull() )
             QMimeSourceFactory::defaultFactory()->setImage( "myimage", picture.data() );
          else {
                   /*
             if ( !picture.url().isEmpty() ) {
                if ( mImageData.count() > 0 )
                   QMimeSourceFactory::defaultFactory()->setImage( "myimage", mImageData );
                else {
                      mImageJob = KIO::get( KURL( picture.url() ), false, false );
                      connect( mImageJob, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
                      this, SLOT( data( KIO::Job*, const QByteArray& ) ) );
                      connect( mImageJob, SIGNAL( result( KIO::Job* ) ),
                      this, SLOT( result( KIO::Job* ) ) );
                }
             } else {
                    */
                QMimeSourceFactory::defaultFactory()->setPixmap( "myimage",
                      KGlobal::iconLoader()->loadIcon( "identity", KIcon::Desktop, 128 ) );
                /*
             }
             */

          }
       }

       writer->queue( "</div></table>" );
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
      return idx == 0 ? "x-vcard" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int ) const { 
       return 0; 
    }
  };

}

extern "C"
KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_vcard_create_bodypart_formatter_plugin() {
  return new Plugin();
}


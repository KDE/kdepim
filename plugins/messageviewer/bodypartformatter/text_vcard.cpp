/*  -*- mode: C++; c-file-style: "gnu" -*-

  This file is part of KMail, the KDE mail client.
  Copyright (c) 2004 Till Adam <adam@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Copyright (c) 2012 Laurent Montel <montel@kde.org>

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
#include "messageviewer/interfaces/bodypartformatter.h"
#include "messageviewer/interfaces/bodyparturlhandler.h"
#include "messageviewer/interfaces/bodypart.h"
#include <messageviewer/nodehelper.h>

using MessageViewer::Interface::BodyPart;
#include "messageviewer/webkitparthtmlwriter.h"

#include <libkdepim/addcontactjob.h>

#include <Akonadi/Contact/ContactViewer>
#include <Akonadi/Contact/StandardContactFormatter>

#include <KABC/VCardConverter>
#include <KABC/Addressee>

#include <kdemacros.h>
#include <KFileDialog>
#include <KGlobal>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KTemporaryFile>
#include <KIO/NetAccess>
#include <QDebug>

namespace {

class Formatter : public MessageViewer::Interface::BodyPartFormatter
{
  public:
    Formatter()
    {
    }

    Result format( BodyPart *bodyPart, MessageViewer::HtmlWriter *writer ) const
    {
      if ( !writer ) {
        return AsIcon;
      }

      KABC::VCardConverter vcc;
      const QString vCard = bodyPart->asText();
      if ( vCard.isEmpty() ) {
        return AsIcon;
      }

      KABC::Addressee::List al = vcc.parseVCards( vCard.toUtf8() );

      // Pre-count the number of non-empty addressees
      int count = 0;
      foreach ( const KABC::Addressee &a, al ) {
        if ( a.isEmpty() ) {
          continue;
        }
        count++;
      }
      if ( !count ) {
        return AsIcon;
      }

      writer->queue( "<div align=\"center\"><h2>" +
                     i18np( "Attached business card", "Attached business cards", count ) +
                     "</h2></div>" );

      count = 0;
      static QString defaultPixmapPath = QLatin1String("file:///") + KIconLoader::global()->iconPath( "user-identity", KIconLoader::Desktop );
      static QString defaultMapIconPath = QLatin1String("file:///") + KIconLoader::global()->iconPath( "document-open-remote", KIconLoader::Small );
      foreach ( const KABC::Addressee &a, al ) {
        if ( a.isEmpty() ) {
          continue;
        }        
        Akonadi::StandardContactFormatter formatter;
        formatter.setContact( a );
        formatter.setDisplayQRCode(false);
        QString htmlStr = formatter.toHtml( Akonadi::StandardContactFormatter::EmbeddableForm );
        KABC::Picture photo = a.photo();
        htmlStr.replace(QLatin1String("<img src=\"map_icon\""),QString::fromLatin1("<img src=\"%1\"").arg(defaultMapIconPath));
        if(photo.isEmpty()) {
          htmlStr.replace(QLatin1String("img src=\"contact_photo\""),QString::fromLatin1("img src=\"%1\"").arg(defaultPixmapPath));
        } else {
          QImage img = a.photo().data();
          const QString dir = bodyPart->nodeHelper()->createTempDir( "vcard-" + a.uid() );
          const QString filename = dir + QDir::separator() + a.uid();
          img.save(filename,"PNG");
          bodyPart->nodeHelper()->addTempFile( filename );
          const QString href = QLatin1String("file:") + KUrl::toPercentEncoding( filename );
          htmlStr.replace(QLatin1String("img src=\"contact_photo\""),QString::fromLatin1("img src=\"%1\"").arg(href));
        }
        writer->queue( htmlStr );

        QString addToLinkText = i18n( "[Add this contact to the address book]" );
        QString op = QString::fromLatin1( "addToAddressBook:%1" ).arg( count );
        writer->queue( "<div align=\"center\"><a href=\"" +
                       bodyPart->makeLink( op ) +
                       "\">" +
                       addToLinkText +
                       "</a></div><br><br>" );
        count++;
      }

      return Ok;
    }
};

class UrlHandler : public MessageViewer::Interface::BodyPartURLHandler
{
  public:
    bool handleClick( MessageViewer::Viewer *viewerInstance, BodyPart *bodyPart,
                      const QString &path ) const
    {

      Q_UNUSED( viewerInstance );
      const QString vCard = bodyPart->asText();
      if ( vCard.isEmpty() ) {
        return true;
      }
      KABC::VCardConverter vcc;
      KABC::Addressee::List al = vcc.parseVCards( vCard.toUtf8() );
      int index = path.right( path.length() - path.lastIndexOf( ":" ) - 1 ).toInt();
      if ( index == -1 || index >= al.count() ) {
        return true;
      }
      KABC::Addressee a = al[index];
      if ( a.isEmpty() ) {
        return true;
      }

      KPIM::AddContactJob *job = new KPIM::AddContactJob( a, 0 );
      job->start();

      return true;
    }

    static KABC::Addressee findAddressee( BodyPart *part, const QString &path )
    {
      const QString vCard = part->asText();
      if ( !vCard.isEmpty() ) {
        KABC::VCardConverter vcc;
        KABC::Addressee::List al = vcc.parseVCards( vCard.toUtf8() );
        const int index = path.right( path.length() - path.lastIndexOf( ":" ) - 1 ).toInt();
        if ( index >= 0 && index < al.count() ) {
          return al[index];
        }
      }
      return KABC::Addressee();
    }

    bool handleContextMenuRequest( BodyPart *part, const QString &path, const QPoint &point ) const
    {
      const QString vCard = part->asText();
      if ( vCard.isEmpty() ) {
        return true;
      }
      KABC::Addressee a = findAddressee( part, path );
      if ( a.isEmpty() ) {
        return true;
      }

      KMenu *menu = new KMenu();
      QAction *open =
        menu->addAction( KIcon( "document-open" ), i18n( "View Business Card" ) );
      QAction *saveas =
        menu->addAction( KIcon( "document-save-as" ), i18n( "Save Business Card As..." ) );

      QAction *action = menu->exec( point, 0 );
      if ( action == open ) {
        openVCard( a, vCard );
      } else if ( action == saveas ) {
        saveAsVCard( a, vCard );
      }
      delete menu;
      return true;
    }

    QString statusBarMessage( BodyPart *part, const QString &path ) const
    {
      KABC::Addressee a = findAddressee( part, path );
      if ( a.realName().isEmpty() ) {
        return i18n( "Add this contact to the address book." );
      } else {
        return i18n( "Add \"%1\" to the address book.", a.realName() );
     }
    }

    bool openVCard( const KABC::Addressee &a, const QString &vCard ) const
    {
      Q_UNUSED( vCard );
      Akonadi::ContactViewer *view = new Akonadi::ContactViewer( 0 );
      view->setRawContact( a );
      view->setMinimumSize( 300, 400 );
      view->show();
      return true;
    }

    bool saveAsVCard( const KABC::Addressee &a, const QString &vCard ) const
    {
      QString fileName;
      const QString givenName(a.givenName());
      if(givenName.isEmpty()) {
        fileName = a.familyName() + QLatin1String(".vcf");
      } else {
        fileName = givenName + QLatin1Char('_') + a.familyName() + QLatin1String(".vcf");
      }
      // get the saveas file name
      KUrl saveAsUrl =
        KFileDialog::getSaveUrl( fileName,
                                 QString(), 0,
                                 i18n( "Save Business Card" ) );
      if ( saveAsUrl.isEmpty() ||
           ( QFileInfo( saveAsUrl.path() ).exists() &&
             ( KMessageBox::warningYesNo(
               0,
               i18n( "%1 already exists. Do you want to overwrite it?",
                     saveAsUrl.path() ) ) == KMessageBox::No ) ) ) {
        return false;
      }

      // put the attachment in a temporary file and save it
      KTemporaryFile tmpFile;
      tmpFile.open();

      QByteArray data = vCard.toUtf8();
      tmpFile.write( data );
      tmpFile.flush();

      return KIO::NetAccess::upload( tmpFile.fileName(), saveAsUrl, 0 );
    }
};

class Plugin : public MessageViewer::Interface::BodyPartFormatterPlugin
{
  public:
    const MessageViewer::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const
    {
      return validIndex( idx ) ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const
    {
      return validIndex( idx ) ? "text" : 0 ;
    }
    const char * subtype( int idx ) const
    {
      switch( idx ) {
      case 0:
        return "x-vcard";
      case 1:
        return "vcard";
      case 2:
        return "directory";
      default:
        return 0;
      }
    }

    const MessageViewer::Interface::BodyPartURLHandler * urlHandler( int idx ) const
    {
       return validIndex( idx ) ? new UrlHandler() : 0 ;
    }

  private:
    bool validIndex( int idx ) const
    {
      return ( idx >= 0 && idx <= 2 );
    }
};

}

extern "C"
KDE_EXPORT MessageViewer::Interface::BodyPartFormatterPlugin *
messageviewer_bodypartformatter_text_vcard_create_bodypart_formatter_plugin()
{
  KGlobal::locale()->insertCatalog( "messageviewer_text_vcard_plugin" );
  return new Plugin();
}


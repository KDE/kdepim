/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QtCore/QFile>
#include <QtGui/QLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>

#include "aboutpage.h"

static QString readFile( const QString &fileName )
{
  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug(5200) <<"Unable to open file '" << fileName <<"'";
    return QString();
  }

  QString content = QString::fromUtf8( file.readAll() );

  file.close();

  return content;
}

AboutPage::AboutPage( QWidget *parent )
  : QWidget( parent )
{
  setObjectName( "AboutPage" );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QString location = KStandardDirs::locate( "data", "kitchensync/about/main.html" );
  QString content = readFile( location );

  content = content.arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage.css" ) );
  if ( QApplication::isRightToLeft() ) {
    content = content.arg( "@import \"%1\";" ).
              arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
  } else {
    content = content.arg( "" );
  }

  KHTMLPart *part = new KHTMLPart( this );
  layout->addWidget( part->view() );

  part->begin( KUrl( location ) );

  QString appName( i18n( "KDE KitchenSync" ) );
  QString catchPhrase( i18n( "Get Synchronized!" ) );
  QString quickDescription( i18n( "The KDE Synchronization Tool" ) );

  part->write( content.arg( QFont().pointSize() + 2 ).arg( appName )
              .arg( catchPhrase ).arg( quickDescription ).arg( htmlText() ) );
  part->end();

  connect( part->browserExtension(),
           SIGNAL( openUrlRequest( const KUrl&, const KParts::OpenUrlArguments&,
                                                const KParts::BrowserArguments& ) ),
           SLOT( handleUrl( const KUrl& ) ) );

  connect( part->browserExtension(),
           SIGNAL( createNewWindow( const KUrl&, const KParts::OpenUrlArguments&,
                                                 const KParts::BrowserArguments&,
                                                 const KParts::WindowArgs&,
                                                 KParts::ReadOnlyPart** ) ),
           SLOT( handleUrl( const KUrl& ) ) );
}

void AboutPage::handleUrl( const KUrl &url )
{
  if ( url.protocol() == "exec" ) {
    if ( url.path() == "/addGroup" ) {
      emit addGroup();
    }
  } else {
    new KRun( url, this );
  }
}

QString AboutPage::htmlText() const
{
  KIconLoader *iconloader = KIconLoader::global();
  int iconSize = iconloader->currentSize( KIconLoader::Desktop );

  QString handbook_icon_path = iconloader->iconPath( "system-help", KIconLoader::Desktop );
  QString html_icon_path = iconloader->iconPath( "applications-internet", KIconLoader::Desktop );
  QString wizard_icon_path = iconloader->iconPath( "tools-wizard", KIconLoader::Desktop );

  QString info = QString( "<h2 style='text-align:center; margin-top: 0px;'>%1 %2</h2>" )
                 .arg( i18n( "Welcome to KitchenSync" ),
                       KGlobal::mainComponent().aboutData()->version() );

  info += QString( "<p>%1</p>" )
          .arg( i18n( "KitchenSync synchronizes your e-mail, address book, calendar, to-do list and more." ) );

  info += QLatin1String( "<table align=\"center\">" );

  info += QString( "<tr><td><a href=\"%1\"><img width=\"%2\" height=\"%3\" src=\"%4\" /></a></td>" )
              .arg( "help:/kitchensync" )
              .arg( iconSize )
              .arg( iconSize )
              .arg( handbook_icon_path );

  info += QString( "<td><a href=\"%1\">%2</a><br><span id=\"subtext\"><nobr>%3</td></tr>" )
              .arg( "help:/kitchensync" )
              .arg( i18n( "Read Manual" ) )
              .arg( i18n( "Learn more about KitchenSync and its components" ) );

  info += QString( "<tr><td><a href=\"%1\"><img width=\"%2\" height=\"%3\" src=\"%4\" /></a></td>" )
              .arg( "http://pim.kde.org" )
              .arg( iconSize )
              .arg( iconSize )
              .arg( html_icon_path );

  info += QString( "<td><a href=\"%1\">%2</a><br><span id=\"subtext\"><nobr>%3</td></tr>" )
              .arg( "http://pim.kde.org" )
              .arg( i18n( "Visit KitchenSync Website" ) )
              .arg( i18n( "Access online resources and tutorials" ) );

  info += QString( "<tr><td><a href=\"%1\"><img width=\"%2\" height=\"%3\" src=\"%4\" /></a></td>" )
              .arg( "exec:/addGroup" )
              .arg( iconSize )
              .arg( iconSize )
              .arg( wizard_icon_path );

  info += QString( "<td><a href=\"%1\">%2</a><br><span id=\"subtext\"><nobr>%3</td></tr>" )
              .arg( "exec:/addGroup" )
              .arg( i18n( "Add Synchronization Group" ) )
              .arg( i18n( "Create group of devices for synchronization" ) );

  info += QLatin1String( "</table>" );

  return info;
}

#include "aboutpage.moc"

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

#include <qfile.h>
#include <qlayout.h>

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
  if ( !file.open( IO_ReadOnly ) ) {
    kdDebug() << "Unable to open file '" << fileName << "'" << endl;
    return QCString();
  }

  QString content = QString::fromUtf8( file.readAll() );

  file.close();

  return content;
}

AboutPage::AboutPage( QWidget *parent )
  : QWidget( parent, "AboutPage" )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QString location = locate( "data", "kitchensync/about/main.html" );
  QString content = readFile( location );
  content = content.arg( locate( "data", "libkdepim/about/kde_infopage.css" ) );
  if ( kapp->reverseLayout() )
    content = content.arg( "@import \"%1\";" ).arg( locate( "data", "libkdepim/about/kde_infopage_rtl.css" ) );
  else
    content = content.arg( "" );

  KHTMLPart *part = new KHTMLPart( this );
  layout->addWidget( part->view() );

  part->begin( KURL( location ) );

  QString appName( i18n( "KDE KitchenSync" ) );
  QString catchPhrase( i18n( "Get Synchronized!" ) );
  QString quickDescription( i18n( "The KDE Synchronization Tool" ) );

  part->write( content.arg( QFont().pointSize() + 2 ).arg( appName )
              .arg( catchPhrase ).arg( quickDescription ).arg( htmlText() ) );
  part->end();

  connect( part->browserExtension(),
           SIGNAL( openURLRequest( const KURL&, const KParts::URLArgs& ) ),
           SLOT( handleUrl( const KURL& ) ) );

  connect( part->browserExtension(),
           SIGNAL( createNewWindow( const KURL&, const KParts::URLArgs& ) ),
           SLOT( handleUrl( const KURL& ) ) );
}

void AboutPage::handleUrl( const KURL &url )
{
  if ( url.protocol() == "exec" ) {
    if ( url.path() == "/addGroup" )
      emit addGroup();
  } else
    new KRun( url, this );
}

QString AboutPage::htmlText() const
{
  KIconLoader *iconloader = KGlobal::iconLoader();
  int iconSize = iconloader->currentSize( KIcon::Desktop );

  QString handbook_icon_path = iconloader->iconPath( "contents2",  KIcon::Desktop );
  QString html_icon_path = iconloader->iconPath( "html",  KIcon::Desktop );
  QString wizard_icon_path = iconloader->iconPath( "wizard",  KIcon::Desktop );

  QString info = i18n( "<h2 style='text-align:center; margin-top: 0px;'>Welcome to KitchenSync %1</h2>"
      "<p>%1</p>"
      "<table align=\"center\">"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "</table>" )
      .arg( kapp->aboutData()->version() )
      .arg( i18n( "KitchenSync synchronizes your e-mail, addressbook, calendar, to-do list and more." ) )
      .arg( "help:/kitchensync" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( handbook_icon_path )
      .arg( "help:/kitchensync" )
      .arg( i18n( "Read Manual" ) )
      .arg( i18n( "Learn more about KitchenSync and its components" ) )
      .arg( "http://pim.kde.org" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( html_icon_path )
      .arg( "http://pim.kde.org" )
      .arg( i18n( "Visit KitchenSync Website" ) )
      .arg( i18n( "Access online resources and tutorials" ) )
      .arg( "exec:/addGroup" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( wizard_icon_path )
      .arg( "exec:/addGroup" )
      .arg( i18n( "Add Synchronization Group" ) )
      .arg( i18n( "Create group of devices for synchronization" ) );

  return info;
}

#include "aboutpage.moc"

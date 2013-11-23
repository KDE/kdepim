/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kdeclarativeapplication.h"
#include "stylesheetloader.h"

#include <kglobalsettings.h>
#include <KDebug>
#include <QFont>
#include <KCmdLineArgs>
#include <QThread>
#include <qplatformdefs.h>

#ifdef KDELIBS_STATIC_LIBS
int staticInitKConfigGroupGui();
#endif

static inline bool runPreApplicationSetup( const KCmdLineOptions & opts ) {

#ifdef _WIN32_WCE
  QThread::currentThread()->setPriority(QThread::HighPriority);
#endif
#ifdef KDELIBS_STATIC_LIBS
  //This is needed to get KConfig working with QColor
  staticInitKConfigGroupGui();
#endif
    KDeclarativeApplicationBase::preApplicationSetup(opts);
    return true; // <-- default value of KApplication(bool) ctor
}

KDeclarativeApplicationBase::KDeclarativeApplicationBase()
    : KUniqueApplication( runPreApplicationSetup( KCmdLineOptions() ) ) // inject some code before KApplication ctor runs
{
    postApplicationSetup();
}

KDeclarativeApplicationBase::KDeclarativeApplicationBase( const KCmdLineOptions & opts )
    : KUniqueApplication( runPreApplicationSetup( opts ) ) // inject some code before KApplication ctor runs
{
    postApplicationSetup();
}

// static
void KDeclarativeApplicationBase::postApplicationSetup()
{
  static bool run = false;

  if ( run ) {
    kDebug() << "called twice";
    return;
  }

  run = true;

  setFont(KGlobalSettings::generalFont());

#ifdef MEEGO_EDITION_HARMATTAN
  // we don't have a native widget style on Maemo6, so our CSS is better than nothing
  StyleSheetLoader::applyStyle( qApp );
#endif

  // make it look more like on the actual device when testing on the desktop
  if ( KCmdLineArgs::parsedArgs()->isSet( "emulate-maemo5" ) )
    emulateMaemo5();
  else if ( KCmdLineArgs::parsedArgs()->isSet( "emulate-maemo6" ) )
    emulateMaemo6();

  KGlobal::locale()->insertCatalog( QLatin1String("libakonadi") );
  KGlobal::locale()->insertCatalog( QLatin1String("accountwizard") );
  KGlobal::locale()->insertCatalog( QLatin1String("libkdepimmobileui") );
  KGlobal::locale()->insertCatalog( QLatin1String("akonadi_serializer_plugins") ); // for conflict handling
}

// static
void KDeclarativeApplicationBase::preApplicationSetup()
{
  preApplicationSetup( KCmdLineOptions() );
}


// static
void KDeclarativeApplicationBase::preApplicationSetup( const KCmdLineOptions & appOptions )
{
  static bool run = false;

  if ( run ) {
    kDebug() << "called twice";
    return;
  }

  run = true;

  // doesn't really belong here, but needs to be called before the ctor
  QApplication::setGraphicsSystem( QLatin1String("raster") );

  KCmdLineOptions options(appOptions);
  options.add("timeit", ki18n("start timers for various parts of the application startup"));
  options.add("enable-opengl", ki18n("use OpenGL ES acceleration for rendering (for testing only)"));
  options.add("disable-opengl", ki18n("do not use OpenGL ES acceleration for rendering (for testing only)"));
  options.add("emulate-maemo5", ki18n("emulate Maemo5 look (for testing only)"));
  options.add("emulate-maemo6", ki18n("emulate Maemo6 look (for testing only)"));
  KCmdLineArgs::addCmdLineOptions(options);
}

void KDeclarativeApplicationBase::emulateMaemo5()
{
  QFont f = font();
  f.setPointSize( 16 );
  setFont( f );

  QPalette p;
  p.setColor( QPalette::Window,          QColor( 0,     0,   0 ) );
  p.setColor( QPalette::WindowText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::Base,            QColor( 255, 255, 255 ) );
  p.setColor( QPalette::AlternateBase,   QColor( 239, 239, 239 ) );
  p.setColor( QPalette::Text,            QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Button,          QColor(   0,   0,   0 ) );
  p.setColor( QPalette::ButtonText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::BrightText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::Light,           QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Midlight,        QColor( 203, 199, 196 ) );
  p.setColor( QPalette::Dark,            QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Mid,             QColor( 184, 181, 178 ) );
  p.setColor( QPalette::Shadow,          QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Highlight,       QColor(  55, 180, 252 ) );
  p.setColor( QPalette::HighlightedText, QColor(   0,  16,  26 ) );
  p.setColor( QPalette::Link,            QColor(   0,   0, 255 ) );
  p.setColor( QPalette::LinkVisited,     QColor( 255,   0, 255 ) );

  p.setColor( QPalette::Disabled, QPalette::WindowText,      QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::Text,            QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::ButtonText,      QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::Highlight,       QColor( 252, 252, 252 ) );
  p.setColor( QPalette::Disabled, QPalette::HighlightedText, QColor(  26,  26,  26 ) );

  setPalette( p );

  setStyle( QLatin1String("plastique") ); // to avoid oxygen artefacts
}

void KDeclarativeApplicationBase::emulateMaemo6()
{
  setStyle( QLatin1String("windows") ); // no kidding, that's apparently the fallback widget style used there...

  StyleSheetLoader::applyStyle( qApp ); // ifdef'ed harmattan-only above
}


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

#include <QDebug>
#include <QFont>
#include <KCmdLineArgs>
#include <QThread>
#include <qplatformdefs.h>
#include <QFontDatabase>

#ifdef KDELIBS_STATIC_LIBS
int staticInitKConfigGroupGui();
#endif

static inline bool runPreApplicationSetup( const KCmdLineOptions & opts ) {

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
    qDebug() << "called twice";
    return;
  }

  run = true;

  setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

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
    qDebug() << "called twice";
    return;
  }

  run = true;

  // doesn't really belong here, but needs to be called before the ctor
  QApplication::setGraphicsSystem( QLatin1String("raster") );

  KCmdLineOptions options(appOptions);
  options.add("timeit", ki18n("start timers for various parts of the application startup"));
  options.add("enable-opengl", ki18n("use OpenGL ES acceleration for rendering (for testing only)"));
  options.add("disable-opengl", ki18n("do not use OpenGL ES acceleration for rendering (for testing only)"));
  KCmdLineArgs::addCmdLineOptions(options);
}


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

#ifndef KDECLARATIVEAPPLICATION_H
#define KDECLARATIVEAPPLICATION_H

#include "mobileui_export.h"
#include <kuniqueapplication.h>
#include <kdebug.h>

class KCmdLineOptions;

class MOBILEUI_EXPORT KDeclarativeApplicationBase : public KUniqueApplication
{
  Q_OBJECT
  public:
    KDeclarativeApplicationBase();
    explicit KDeclarativeApplicationBase( const KCmdLineOptions & applicationOptions );

    /** Sets up some stuff. Only needs to be called (before the
        KApplication constructor) if you don't use
        KDeclarativeApplication as your KApplication.
        You can pass your own options as \a applicationOptions.
    */
    static void preApplicationSetup( const KCmdLineOptions & applicationOptions );
    /**
       \overload
    */
    static void preApplicationSetup();

    KDE_DEPRECATED static void initCmdLine() { preApplicationSetup(); }

    /** Sets up some other stuff. Only needs to be called (after the
        KApplication constructor) if you don't use
        KDeclarativeApplication as your KApplication */
    static void postApplicationSetup();

  private:
    static void emulateMaemo5();
    static void emulateMaemo6();
};

template <typename T>
class KDeclarativeApplication : public KDeclarativeApplicationBase
{
  public:
    KDeclarativeApplication() : KDeclarativeApplicationBase(), m_mainView( 0 ) {}
    explicit KDeclarativeApplication( const KCmdLineOptions &applicationOptions ) : KDeclarativeApplicationBase( applicationOptions ), m_mainView( 0 ) {}
    virtual ~KDeclarativeApplication()
    {
      delete m_mainView;
    }

    int newInstance()
    {
      kDebug();
      if ( !m_mainView ) {
        m_mainView = new T;
        m_mainView->show();
      } else {
        m_mainView->raise();
      }

      return 0;
    }

  protected:
    T* m_mainView;
};

#endif

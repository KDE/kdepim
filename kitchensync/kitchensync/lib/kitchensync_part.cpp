/*
    This file is part of KitchenSync.

    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kitchensync_part.h"

#include "actionmanager.h"
#include "kitchensync.h"

#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kkeydialog.h>

#include <qapplication.h>
#include <qfile.h>

extern "C"
{
  /**
   * This function is the 'main' function of this part.  It takes
   * the form 'void *init_lib<library name>()  It always returns a
   * new factory object
   */
  void *init_libkitchensyncpart()
  {
    return new KitchenSyncFactory;
  }
}

/**
* We need one static instance of the factory for our C 'main'
* function
*/
KInstance *KitchenSyncFactory::s_instance = 0;
KAboutData *KitchenSyncFactory::s_about = 0;

KitchenSyncFactory::KitchenSyncFactory()
{
}

KitchenSyncFactory::~KitchenSyncFactory()
{
  delete s_instance;
  s_instance = 0;
  delete s_about;
}

KParts::Part *KitchenSyncFactory::createPartObject(QWidget *parentWidget, const char *widgetName,
                                   QObject *parent, const char *name,
                                   const char*,const QStringList& )
{
  KParts::Part *obj = new KitchenSyncPart(parentWidget, widgetName, parent, name );
  return obj;
}

KInstance *KitchenSyncFactory::instance()
{
  if ( !s_instance ) {
    s_about = new KAboutData("kitchensync", I18N_NOOP("KitchenSync"),"0.1");
    s_instance = new KInstance(s_about);
  }

  kdDebug(5850) << "KitchenSyncFactory::instance(): Name: " <<
               s_instance->instanceName() << endl;

  return s_instance;
}

KitchenSyncPart::KitchenSyncPart( QWidget *parentWidget, const char *,
                                  QObject *parent, const char *name ) :
  KParts::ReadOnlyPart( parent, name )
{
  QString pname( name );

  mActionManager = new KSync::ActionManager( actionCollection() );
  
  KSync::KitchenSync *view = new KSync::KitchenSync( mActionManager,
                                                     parentWidget );

  mActionManager->setView( view );
  mActionManager->initActions();

  setWidget( view );

  view->initProfiles();
  view->slotProfile();

  setInstance( KitchenSyncFactory::instance() );

  setXMLFile("ksyncgui.rc");
}

KitchenSyncPart::~KitchenSyncPart()
{
  delete mActionManager;
}

bool KitchenSyncPart::openFile()
{
  return true;
}

using namespace KParts;

#include "kitchensync_part.moc"



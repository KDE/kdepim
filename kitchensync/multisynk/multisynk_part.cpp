/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kparts/genericfactory.h>
#include <kstatusbar.h>

#include "mainwidget.h"

#include "multisynk_part.h"

typedef KParts::GenericFactory< MultiSynkPart > MultiSynkFactory;
K_EXPORT_COMPONENT_FACTORY( libmultisynkpart, MultiSynkFactory )

MultiSynkPart::MultiSynkPart( QWidget *parentWidget, const char *widgetName,
                              QObject *parent, const char *name,
                              const QStringList & )
  : KParts::ReadOnlyPart( parent, name )
{
  setInstance( MultiSynkFactory::instance() );

  // create a canvas to insert our widget
  MainWidget *wdg = new MainWidget( this, parentWidget, widgetName );
  setWidget( wdg );

  KGlobal::iconLoader()->addAppDir( "multisynk" );

  setXMLFile( "multisynk_part.rc" );
}

MultiSynkPart::~MultiSynkPart()
{
  closeURL();
}

KAboutData *MultiSynkPart::createAboutData()
{
  return MainWidget::aboutData();
}

void MultiSynkPart::exit()
{
  delete this;
}

bool MultiSynkPart::openURL( const KURL &url )
{
  emit setWindowCaption( url.prettyURL() );

  return true;
}

bool MultiSynkPart::openFile()
{
  return true;
}

void MultiSynkPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
  KParts::ReadOnlyPart::guiActivateEvent( e );
}

#include "multisynk_part.moc"


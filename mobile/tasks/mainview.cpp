/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

MainView::MainView( QWidget *parent ) : QDeclarativeView( parent )
{
  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );

  const QString qmlPath = KStandardDirs::locate( "data", "mobile/tasks.qml" );
  kDebug() << qmlPath;
  setSource( qmlPath );
}

/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#include "debugger.h"

#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <mainwindow.h>

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>

#include <qlabel.h>

typedef KParts::GenericFactory< KSync::Debugger> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_debugger, DebuggerFactory );

using namespace KSync ;

Debugger::Debugger( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("package_settings", KIcon::Desktop, 48 );
}

KAboutData *Debugger::createAboutData()
{
  return new KAboutData("KSyncDebugger", I18N_NOOP("Sync Debugger Part"), "0.0" );
}

Debugger::~Debugger()
{
  delete m_widget;
}

QString Debugger::type() const
{
  return QString::fromLatin1("debugger");
}

QString Debugger::name() const
{
  return i18n("Konnector Debugger");
}

QString Debugger::description() const
{
  return i18n("Debugger for Konnectors");
}

QPixmap *Debugger::pixmap()
{
  return &m_pixmap;
}

QString Debugger::iconName() const
{
  return QString::fromLatin1("kcmsystem");
}

bool Debugger::partIsVisible() const
{
  return true;
}

QWidget *Debugger::widget()
{
  if( !m_widget ) {
    m_widget = new QLabel( "Hallo", 0 );
  }
  return m_widget;
}

#include "debugger.moc"

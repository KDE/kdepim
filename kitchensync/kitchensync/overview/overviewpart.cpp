/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <kiconloader.h>
#include <kparts/genericfactory.h>

#include <konnector.h>
#include <konnectorinfo.h>
#include <konnectormanager.h>

#include <mainwindow.h>

#include "overviewwidget.h"

#include "overviewpart.h"

typedef KParts::GenericFactory< KSync::OverviewPart> OverviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( liboverviewpart, OverviewPartFactory )

using namespace KSync;

OverviewPart::OverviewPart( QWidget *parent, const char *name,
                            QObject *, const char *,const QStringList & )
  : ActionPart( parent, name )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon( "kcmsystem", KIcon::Desktop, 48 );
  m_widget=0;

  connectPartChange( SLOT( slotPartChanged( ActionPart* ) ) );
  connectProfileChanged( SLOT( slotProfileChanged( const Profile& ) ) );
  connectSyncProgress( SLOT( slotSyncProgress( ActionPart*, int, int ) ) );
  connectStartSync( SLOT( slotStartSync() ) );
  connectDoneSync( SLOT( slotDoneSync() ) );
}

OverviewPart::~OverviewPart()
{
  delete m_widget;
}

KAboutData *OverviewPart::createAboutData()
{
  return new KAboutData( "KSyncOverviewPart", I18N_NOOP( "Sync Overview Part" ), "0.0" );
}

QString OverviewPart::type() const
{
  return QString::fromLatin1( "Overview" );
}

QString OverviewPart::title() const
{
  return i18n( "Overview" );
}

QString OverviewPart::description() const
{
  return i18n( "This part is the main widget of KitchenSync" );
}

QPixmap* OverviewPart::pixmap()
{
  return &m_pixmap;
}

QString OverviewPart::iconName() const
{
  return QString::fromLatin1( "kcmsystem" );
}

bool OverviewPart::hasGui() const
{
  return true;
}

QWidget* OverviewPart::widget()
{
  if ( !m_widget )
    m_widget = new OverView::Widget( 0, "part" );

  return m_widget;
}

void OverviewPart::slotPartChanged( ActionPart* part )
{
  kdDebug(5210) << "PartChanged" << part << " name" << part->name() << endl;
}

void OverviewPart::slotProfileChanged( const Profile & )
{
  m_widget->setProfile( core()->currentProfile() );
  kdDebug(5210) << "Profile changed " << endl;
}

void OverviewPart::slotSyncProgress( ActionPart* part, int status, int percent )
{
  m_widget->syncProgress( part, status, percent );
}

void OverviewPart::slotStartSync()
{
  m_widget->startSync();
  kdDebug(5210) << "Start Sync " << endl;
}

void OverviewPart::slotDoneSync()
{
  kdDebug(5210) << "Done Sync " << endl;
}

void OverviewPart::executeAction()
{
  kdDebug(5210) << "OverviewPart::executeAction()" << endl;
}

#include "overviewpart.moc"

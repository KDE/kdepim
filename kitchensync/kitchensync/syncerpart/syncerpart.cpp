/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "syncerpart.h"

#include "calendarsyncee.h"
#include "addressbooksyncee.h"

#include <konnectorview.h>
#include <syncuikde.h>
#include <konnector.h>
#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <mainwindow.h>
#include <engine.h>

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kdialogbase.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>


typedef KParts::GenericFactory< KSync::SyncerPart> SyncerPartFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_syncerpart, SyncerPartFactory )

using namespace KCal;
using namespace KSync;

SyncerPart::SyncerPart( QWidget *parent, const char *name,
                        QObject *, const char *, const QStringList & )
  : ActionPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon( "package_toys", KIcon::Desktop,
                                              48 );

  /* confirm delete will be changed later */
  mSyncUi = new SyncUiKde( parent, true, true );

  mCalendarSyncer.setSyncUi( mSyncUi );
  mAddressBookSyncer.setSyncUi( mSyncUi );
}

KAboutData *SyncerPart::createAboutData()
{
  return new KAboutData( "KSyncSyncerPart", I18N_NOOP("Sync SyncerPart Part"),
                         "0.0" );
}

SyncerPart::~SyncerPart()
{
  delete m_widget;

  delete mSyncUi;
}

QString SyncerPart::type() const
{
  return QString::fromLatin1("SyncerPart");
}

QString SyncerPart::title() const
{
  return i18n("Synchronizer");
}

QString SyncerPart::description() const
{
  return i18n("Synchronizer");
}

QPixmap *SyncerPart::pixmap()
{
  return &m_pixmap;
}

QString SyncerPart::iconName() const
{
  return QString::fromLatin1("kcmsystem");
}

bool SyncerPart::hasGui() const
{
  return true;
}

QWidget *SyncerPart::widget()
{
  if( !m_widget ) {
    m_widget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( m_widget );
    topLayout->setSpacing( KDialog::spacingHint() );


    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    mKonnectorView = new KonnectorView( m_widget );
    konnectorLayout->addWidget( mKonnectorView, 1 );

    QFrame *konnectorFrame = new QFrame( m_widget );
    konnectorFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    konnectorLayout->addWidget( konnectorFrame, 1 );


    mLogView = new QTextView( m_widget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );

    logMessage( i18n("Ready.") );
  }
  return m_widget;
}

// FIXME: Move logging of all parts to common class
void SyncerPart::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  mLogView->append( text );
}

void SyncerPart::executeAction()
{
  logMessage( i18n("Sync Action triggered") );

  mCalendarSyncer.clear();
  mAddressBookSyncer.clear();

  /* set confirmDelete according user preference */
  mSyncUi->setConfirmDelete( core()->currentProfile().confirmDelete() );

  Konnector::List konnectors = core()->engine()->konnectors();
  Konnector *k;
  for( k = konnectors.first(); k; k = konnectors.next() ) {
    SynceeList syncees = k->syncees();

    if ( syncees.count() == 0 ) {
      logMessage( i18n("Syncee list is empty.") );
      continue;
    }

    CalendarSyncee *calendarSyncee = syncees.calendarSyncee();
    if ( calendarSyncee ) mCalendarSyncer.addSyncee( calendarSyncee );

    AddressBookSyncee *addressBookSyncee = syncees.addressBookSyncee();
    if ( addressBookSyncee ) mAddressBookSyncer.addSyncee( addressBookSyncee );
  }

  logMessage( i18n("Performing Sync") );

  mCalendarSyncer.sync();
  mAddressBookSyncer.sync();

  logMessage( i18n("Sync done") );
}

#include "syncerpart.moc"

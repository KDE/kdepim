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

#include "syncerpart.h"

#include <konnectorplugin.h>
#include <configwidget.h>
#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <mainwindow.h>
#include <calendarsyncee.h>

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
K_EXPORT_COMPONENT_FACTORY( libksync_syncerpart, SyncerPartFactory );

using namespace KCal;
using namespace KSync;

class KonnectorCheckItem : public QCheckListItem
{
  public:
    KonnectorCheckItem( Konnector *k, QListView *l )
      : QCheckListItem( l, k ? k->info().name() : QString::null, CheckBox ),
        mKonnector( k )
    {
    }

    Konnector *konnector() const { return mKonnector; }

  private:
    Konnector *mKonnector;
};

SyncerPart::SyncerPart( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("package_toys", KIcon::Desktop, 48 );
}

KAboutData *SyncerPart::createAboutData()
{
  return new KAboutData("KSyncSyncerPart", I18N_NOOP("Sync SyncerPart Part"), "0.0" );
}

SyncerPart::~SyncerPart()
{
  delete m_widget;
}

QString SyncerPart::type() const
{
  return QString::fromLatin1("SyncerPart");
}

QString SyncerPart::name() const
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

bool SyncerPart::partIsVisible() const
{
  return true;
}

QWidget *SyncerPart::widget()
{
  if( !m_widget ) {
    m_widget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( m_widget );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( KDialog::spacingHint() );

    
    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    mKonnectorList = new QListView( m_widget );
    mKonnectorList->addColumn( i18n("Konnector" ) );
    konnectorLayout->addWidget( mKonnectorList, 1 );

    updateKonnectorList();

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

void SyncerPart::updateKonnectorList()
{
  KonnectorProfile::ValueList konnectors =
      core()->konnectorProfileManager()->list();

  KonnectorProfile::ValueList::ConstIterator it;
  for( it = konnectors.begin(); it != konnectors.end(); ++it ) {
    QCheckListItem *item = new KonnectorCheckItem( 0, mKonnectorList );
    item->setText( 0, (*it).name() );
  }
}

void SyncerPart::slotProgress( Konnector *k, const Progress &p )
{
  logMessage( i18n("Got Progress from Konnector at address %1: %2").arg( (long)k ).arg( p.text() ) );
}

void SyncerPart::slotError( Konnector *k, const Error &e )
{
  logMessage( i18n("Got Progress from Konnector at address %1: %2").arg( (long)k ).arg( e.text() ) );
}

void SyncerPart::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  mLogView->append( text );
}

#include "syncerpart.moc"

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

#include <konnector.h>
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
#include <kresources/configdialog.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qvbox.h>

using namespace KCal;
using namespace KSync;

typedef KParts::GenericFactory< Debugger> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_debugger, DebuggerFactory )


class CustomComboBox : public QComboBox
{
  public:
    CustomComboBox( QWidget *parent, const char *name = 0 )
      : QComboBox( parent, name ) {}

    void insertItem( Konnector *k, const QString &text )
    {
      QComboBox::insertItem( text );
      mKonnectors.append( k );
    }

    Konnector *currentKonnector()
    {
      return mKonnectors.at( currentItem() );
    }

  private:
    QPtrList<Konnector> mKonnectors;
};


Debugger::Debugger( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ActionPart( parent, name ), m_widget( 0 ),
    mCalendar( QString::fromLatin1( "UTC" ) )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("package_settings", KIcon::Desktop, 48 );

  Event *event = new Event;
  event->setSummary( "Debugger Event" );
  mCalendar.addEvent( event );
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

QString Debugger::title() const
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

bool Debugger::hasGui() const
{
  return true;
}

QWidget *Debugger::widget()
{
  if( !m_widget ) {
    m_widget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( m_widget );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( KDialog::spacingHint() );


    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    konnectorLayout->addWidget( new QLabel( i18n("Current Konnector:" ),
                                            m_widget ) );

    mKonnectorCombo = new CustomComboBox( m_widget );
    konnectorLayout->addWidget( mKonnectorCombo );

    updateKonnectors();

    konnectorLayout->addStretch();


    QBoxLayout *commandLayout = new QHBoxLayout( topLayout );

    QPushButton *button = new QPushButton( "Configure...", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( configureKonnector() ) );
    commandLayout->addWidget( button );

    button = new QPushButton( "Connect Device", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( connectDevice() ) );
    commandLayout->addWidget( button );

    button = new QPushButton( "Read Syncees", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( readSyncees() ) );
    commandLayout->addWidget( button );

    button = new QPushButton( "Write Syncees", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( writeSyncees() ) );
    commandLayout->addWidget( button );

    button = new QPushButton( "Disconnect Device", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( disconnectDevice() ) );
    commandLayout->addWidget( button );


    commandLayout->addStretch();


    mLogView = new QTextView( m_widget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );

    logMessage( i18n("Ready.") );
  }
  return m_widget;
}

void Debugger::updateKonnectors()
{
  kdDebug() << "Debugger::updateKonnectors()" << endl;

  KRES::Manager<Konnector> *manager = KonnectorManager::self();

  KRES::Manager<Konnector>::ActiveIterator it;
  for( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    kdDebug() << "Konnector: id: " << (*it)->identifier() << endl;
    mKonnectorCombo->insertItem( *it, (*it)->resourceName() );
  }
}

void Debugger::configureKonnector()
{
  Konnector *k = currentKonnector();
  if ( !k ) {
    KMessageBox::sorry( m_widget, i18n("Konnector isn't loaded" ) );
  } else {
    KRES::ConfigDialog *dialog = new KRES::ConfigDialog( m_widget, "konnector",
                                                         k );
    if ( !dialog ) {
      KMessageBox::sorry( m_widget,
                          i18n("No configuration widget available.") );
    } else {
      dialog->show();
    }
  }
}

Konnector *Debugger::currentKonnector()
{
  Konnector *k = mKonnectorCombo->currentKonnector();

  if ( mConnectedKonnectors.find( k ) < 0 ) {
    kdDebug() << "Connect Konnector" << endl;
    connect( k, SIGNAL( synceesRead( KSync::Konnector * ) ),
             SLOT( slotReceiveData( KSync::Konnector * ) ) );
    mConnectedKonnectors.append( k );
  }

  return k;
}

void Debugger::readSyncees()
{
  logMessage( i18n("Read Syncees") );

  Konnector *k = currentKonnector();

  if ( k ) k->readSyncees();
}

void Debugger::slotReceiveData( Konnector *k )
{
  logMessage( i18n("Got Syncee list from Konnector at address %1").arg( (long)k ) );
  mSynceeList = k->syncees();

  SynceeList::ConstIterator it;
  for( it = mSynceeList.begin(); it != mSynceeList.end(); ++it ) {
    Syncee *syncee = *it;
    logMessage( i18n("Got Syncee of type %1").arg( syncee->type() ) );
    SyncEntry *syncEntry;
    int i = 0;
    for( syncEntry = syncee->firstEntry(); syncEntry;
         syncEntry = syncee->nextEntry() ) {
      logMessage( " " + syncEntry->id() + ": " + syncEntry->name() );
      ++i;
    }
    if ( i == 0 ) logMessage( i18n(" Empty") );
  }
}

void Debugger::writeSyncees()
{
  KDialogBase dialog( m_widget, 0, true, i18n("Select Syncees"),
                      KDialogBase::Ok | KDialogBase::Cancel );
  QVBox *topBox = dialog.makeVBoxMainWidget();
  QCheckBox mEventCheck( i18n("Events"), topBox );
  mEventCheck.setChecked( true );
  QCheckBox mAddresseeCheck( i18n("Addressees"), topBox );
  mAddresseeCheck.setChecked( true );
  int result = dialog.exec();
  if ( result == QDialog::Accepted ) {
    logMessage( i18n("Write Syncees") );
    if ( mEventCheck.isChecked() ) {
      logMessage( i18n("Write events") );
      CalendarSyncee *calendarSyncee = mSynceeList.calendarSyncee();
      if ( !calendarSyncee ) {
        logMessage( i18n("No calendar syncee.") );
      } else {
        Calendar *cal = calendarSyncee->calendar();
        Event *e = new Event();
        e->setSummary( "Debugger was here (" + QTime::currentTime().toString()
                       + ")" );
        cal->addEvent( e );
      }
    }
    if ( mAddresseeCheck.isChecked() ) {
      logMessage( i18n("Write Addressees") );
      kdDebug() << "To be implemented: Create debugger addressee syncee."
                << endl;
    }
    kdDebug() << "Send data" << endl;
    Konnector *k = currentKonnector();
    if ( k ) k->writeSyncees();
  }
}

void Debugger::connectDevice()
{
  logMessage( i18n("Connecting to Device.") );

  Konnector *k = currentKonnector();
  if ( k ) k->connectDevice();
}

void Debugger::disconnectDevice()
{
  logMessage( i18n("Disconnecting from Device.") );

  Konnector *k = currentKonnector();
  if ( k ) k->disconnectDevice();
}

void Debugger::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  kdDebug() << "LOG: " << text << endl;

  mLogView->append( text );
}

void Debugger::executeAction()
{
  logMessage( i18n("actionSync()") );
}

#include "debugger.moc"

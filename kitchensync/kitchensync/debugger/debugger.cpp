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
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qvbox.h>

typedef KParts::GenericFactory< KSync::Debugger> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_debugger, DebuggerFactory );

using namespace KCal;
using namespace KSync ;

Debugger::Debugger( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
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
    m_widget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( m_widget );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( KDialog::spacingHint() );

    
    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    konnectorLayout->addWidget( new QLabel( i18n("Current Konnector:" ),
                                            m_widget ) );

    mKonnectorCombo = new QComboBox( m_widget );
    konnectorLayout->addWidget( mKonnectorCombo );

    KonnectorProfile::ValueList konnectors =
        core()->konnectorProfileManager()->list();

    KonnectorProfile::ValueList::ConstIterator it;
    for( it = konnectors.begin(); it != konnectors.end(); ++it ) {
      mKonnectorCombo->insertItem( (*it).name() );
    }

    konnectorLayout->addStretch();


    QBoxLayout *commandLayout = new QHBoxLayout( topLayout );

    QPushButton *button = new QPushButton( "Configure...", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( configureKonnector() ) );    
    commandLayout->addWidget( button );

    button = new QPushButton( "Read Syncees", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( readSyncees() ) );
    commandLayout->addWidget( button );

    button = new QPushButton( "Write Syncees", m_widget );
    connect( button, SIGNAL( clicked() ), SLOT( writeSyncees() ) );
    commandLayout->addWidget( button );

    commandLayout->addStretch();


    mLogView = new QTextView( m_widget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );    

    logMessage( i18n("Ready.") );
  }
  return m_widget;
}

void Debugger::configureKonnector()
{
  Konnector *k = currentKonnector();
  if ( !k ) {
    KMessageBox::sorry( m_widget, i18n("Konnector isn't loaded" ) );
  } else {
    KDialog *dialog = new KDialog( m_widget );
    ConfigWidget *configWidget = k->configWidget( dialog );
    if ( !configWidget ) {
      KMessageBox::sorry( m_widget,
                          i18n("No configuration widget available.") );
    } else {
      QVBoxLayout *dialogLayout = new QVBoxLayout( dialog );
      dialogLayout->addWidget( configWidget );
      dialog->show();
    }
  }
}

Konnector *Debugger::currentKonnector()
{
  QString konnectorName = mKonnectorCombo->currentText();
  
  KonnectorProfile::ValueList konnectors =
      core()->konnectorProfileManager()->list();

  KonnectorProfile::ValueList::Iterator it;
  for( it = konnectors.begin(); it != konnectors.end(); ++it ) {
    if ( konnectorName == (*it).name() ) break;
  }

  if ( it == konnectors.end() ) return 0;

  if ( !(*it).konnector() ) {
    kdDebug() << "Create Konnector" << endl;
    Konnector *k = core()->konnectorManager()->load( (*it).device() );
    connect( k, SIGNAL( sync( Konnector *, Syncee::PtrList ) ),
             SLOT( slotReceiveData( Konnector *, Syncee::PtrList ) ) );
    return k;
  }

  return (*it).konnector();
}

void Debugger::readSyncees()
{
  logMessage( i18n("Read Syncees") );

  Konnector *k = currentKonnector();
  
  if ( k ) k->startSync();
}

void Debugger::slotReceiveData( Konnector *, Syncee::PtrList syncees )
{
  Syncee *syncee;
  for( syncee = syncees.first(); syncee; syncee = syncees.next() ) {
    logMessage( i18n("Got Syncee of type %1").arg( syncee->type() ) );
    SyncEntry *syncEntry;
    for( syncEntry = syncee->firstEntry(); syncEntry;
         syncEntry = syncee->nextEntry() ) {
      logMessage( syncEntry->id() + ": " + syncEntry->name() );
    }
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
    Syncee::PtrList syncees;
    if ( mEventCheck.isChecked() ) {
      logMessage( i18n("Write events") );
      syncees.append( new CalendarSyncee( &mCalendar ) );
    }
    if ( mAddresseeCheck.isChecked() ) {
      logMessage( i18n("Write Addressees") );
      kdDebug() << "To be implemented: Create debugger addressee syncee."
                << endl;
    }
    kdDebug() << "Send data" << endl;
    Konnector *k = currentKonnector();
    if ( k ) k->doWrite( syncees );
  }
}

void Debugger::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  mLogView->append( text );
}

#include "debugger.moc"

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

#include "backup.h"

#include "konnectorview.h"
#include "backupview.h"

#include <konnector.h>
#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <mainwindow.h>
#include <calendarsyncee.h>
#include <engine.h>

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qdir.h>

using namespace KCal;
using namespace KSync;

typedef KParts::GenericFactory< Backup> BackupFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_backup, BackupFactory )

Backup::Backup( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ActionPart( parent, name ), mWidget( 0 ), mActive( false )
{
  mPixmap = KGlobal::iconLoader()->loadIcon("kcmdrkonqi", KIcon::Desktop, 48 );
}

KAboutData *Backup::createAboutData()
{
  return new KAboutData("KSyncBackup", I18N_NOOP("Sync Backup Part"), "0.0" );
}

Backup::~Backup()
{
  delete mWidget;
}

QString Backup::type() const
{
  return QString::fromLatin1("backup");
}

QString Backup::title() const
{
  return i18n("Konnector Backup");
}

QString Backup::description() const
{
  return i18n("Backup for Konnectors");
}

QPixmap *Backup::pixmap()
{
  return &mPixmap;
}

QString Backup::iconName() const
{
  return QString::fromLatin1("kcmsystem");
}

bool Backup::hasGui() const
{
  return true;
}

QWidget *Backup::widget()
{
  if( !mWidget ) {
    mWidget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( mWidget );
    topLayout->setSpacing( KDialog::spacingHint() );
    
    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    mKonnectorList = new KonnectorView( mWidget );
    konnectorLayout->addWidget( mKonnectorList, 1 );

    mKonnectorList->updateKonnectorList();

    mBackupView = new BackupView( mWidget );
    konnectorLayout->addWidget( mBackupView );
    connect( mBackupView, SIGNAL( backupDeleted( const QString & ) ),
             SLOT( slotBackupDeleted( const QString & ) ) );
    
    mBackupView->updateBackupList();

    mLogView = new QTextView( mWidget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );

    logMessage( i18n("Ready.") );
  }
  return mWidget;
}

void Backup::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  kdDebug() << "LOG: " << text << endl;

  mLogView->append( text );
}

void Backup::executeAction()
{
  logMessage( i18n("Starting backup") );

  mBackupView->createBackupDir();

  Konnector::List konnectors = core()->engine()->konnectors();
  Konnector *k;
  for( k = konnectors.first(); k; k = konnectors.next() ) {
    backupKonnector( k );
  }

  logMessage( i18n("Backup finished.") );

  mBackupView->updateBackupList();
}

void Backup::backupKonnector( Konnector *k )
{
  logMessage( i18n("Syncees read from '%1'").arg( k->resourceName() ) );

  SynceeList syncees = k->syncees();

  if ( syncees.count() == 0 ) {
    logMessage( i18n("Syncee list is empty.") );
  } else {
    logMessage( i18n("Performing backup.") );
  
    SynceeList::ConstIterator it;
    for( it = syncees.begin(); it != syncees.end(); ++it ) {
      if ( !(*it)->isValid() ) continue;
      QString filename = mBackupView->backupFile( k, *it );
      kdDebug() << "FILENAME: " << filename << endl;
      QString type = (*it)->type();
      if ( (*it)->writeBackup( filename ) ) {
        logMessage( i18n("Wrote backup for %1.").arg( type ) );
      } else {
        logMessage( i18n("<b>Error:</b> Can't write backup for %1.")
                    .arg( type ) );
      }
    }
  }
}

void Backup::slotBackupDeleted( const QString &name )
{
  logMessage( i18n("Backup '%1' deleted").arg( name ) );
}

#include "backup.moc"

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

#include <konnector.h>
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

class KonnectorCheckItem : public QCheckListItem
{
  public:
    KonnectorCheckItem( Konnector *k, QListView *l )
      : QCheckListItem( l, k->resourceName(), CheckBox ),
        mKonnector( k )
    {
    }

    Konnector *konnector() const { return mKonnector; }

  private:
    Konnector *mKonnector;
};

class BackupItem : public QListViewItem
{
  public:
    BackupItem( QListView *parent, const QString &dirName )
      : QListViewItem( parent )
    {
      QDateTime dt = QDateTime::fromString( dirName, ISODate );
      QString txt;
      if ( dt.isValid() ) {
        txt = KGlobal::locale()->formatDateTime( dt );
        mDirName = dirName;
      } else {
        txt = i18n("Invalid (\"%1\")").arg( dirName );
      }
      setText( 0, txt );
    }
    
    QString dirName() const { return mDirName; }
    
  private:
    QString mDirName;
};

Backup::Backup( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 ), mActive( false )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmdrkonqi", KIcon::Desktop, 48 );
}

KAboutData *Backup::createAboutData()
{
  return new KAboutData("KSyncBackup", I18N_NOOP("Sync Backup Part"), "0.0" );
}

Backup::~Backup()
{
  delete m_widget;
}

QString Backup::type() const
{
  return QString::fromLatin1("backup");
}

QString Backup::name() const
{
  return i18n("Konnector Backup");
}

QString Backup::description() const
{
  return i18n("Backup for Konnectors");
}

QPixmap *Backup::pixmap()
{
  return &m_pixmap;
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

    QBoxLayout *restoreLayout = new QVBoxLayout( konnectorLayout );

    mRestoreView = new QListView( m_widget );
    mRestoreView->addColumn( i18n("Backup") );
    restoreLayout->addWidget( mRestoreView, 1 );

    updateRestoreList();

    QPushButton *button = new QPushButton( i18n("Restore"), m_widget );
    restoreLayout->addWidget( button );
    connect( button, SIGNAL( clicked() ), SLOT( restoreBackup() ) );

    button = new QPushButton( i18n("Delete"), m_widget );
    restoreLayout->addWidget( button );
    connect( button, SIGNAL( clicked() ), SLOT( deleteBackup() ) );

    mLogView = new QTextView( m_widget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );

    logMessage( i18n("Ready.") );
  }
  return m_widget;
}

void Backup::updateKonnectorList()
{
  kdDebug() << "Backup::updateKonnectorList()" << endl;

  KRES::Manager<Konnector> *manager = KonnectorManager::self();
  
  KRES::Manager<Konnector>::ActiveIterator it;
  for( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    kdDebug() << "Konnector: id: " << (*it)->identifier() << endl;
    KonnectorCheckItem *item = new KonnectorCheckItem( *it, mKonnectorList );
    item->setOn( true );
  }
}

void Backup::updateRestoreList()
{
  mRestoreView->clear();

  QString dirName = locateLocal( "appdata", topBackupDir() );
  QDir dir( dirName );

  QStringList backups = dir.entryList( QDir::Dirs );
  
  QStringList::ConstIterator it;
  for( it = backups.begin(); it != backups.end(); ++it ) {
    if ( *it != "." && *it != ".." ) {
      new BackupItem( mRestoreView, *it );
    }
  }
}

void Backup::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  kdDebug() << "LOG: " << text << endl;

  mLogView->append( text );
}

void Backup::actionSync()
{
  if ( mActive ) {
    KMessageBox::sorry( m_widget, i18n("Already active.") );
    return;
  }

  mActive = true;
  
  logMessage( i18n("Starting backup") );

  createBackupDir();

  openKonnectors();

  Konnector *k;
  for ( k = mOpenedKonnectors.first(); k; k = mOpenedKonnectors.next() ) {
    logMessage( i18n("Request Syncees") );
    if ( !k->readSyncees() ) {
      logMessage( i18n("Request failed.") );
    }
  }

  tryFinishBackup();
}

QString Backup::topBackupDir() const
{
  return "backup/";
}

void Backup::createBackupDir()
{
  QString date = QDateTime::currentDateTime().toString( ISODate );
  mBackupDir = locateLocal( "appdata", topBackupDir() + date + "/", true );

  kdDebug() << "DIRNAME: " << mBackupDir << endl;
}

QString Backup::backupFile( Konnector *k, Syncee *s )
{
  return mBackupDir + "/" + k->identifier() + "-" + s->type();
}

void Backup::slotSynceesRead( Konnector *k )
{
  logMessage( i18n("Syncees read from '%1'").arg( k->resourceName() ) );

  SynceeList syncees = k->syncees();

  if ( syncees.count() == 0 ) {
    logMessage( i18n("Syncee list is empty.") );
  } else {
    logMessage( i18n("Performing backup.") );
  
    SynceeList::ConstIterator it;
    for( it = syncees.begin(); it != syncees.end(); ++it ) {
      QString filename = backupFile( k, *it );
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

  tryFinishBackup();
}

void Backup::slotSynceeReadError( Konnector *k )
{
  logMessage( i18n("Error reading %1").arg( k->resourceName() ) );
  
  tryFinishBackup();
}

void Backup::tryFinishBackup()
{
  mKonnectorCount--;
  if ( mKonnectorCount > 0 ) return;
  
  logMessage( i18n("Backup finished.") );

  updateRestoreList();

  mActive = false;
}

void Backup::restoreBackup()
{
  if ( mActive ) {
    KMessageBox::sorry( m_widget, i18n("Already active.") );
    return;
  }

  BackupItem *backupItem =
    static_cast<BackupItem *>( mRestoreView->currentItem() );
  
  if ( !backupItem ) {
    KMessageBox::sorry( m_widget, i18n("No backup selected.") );
    return;
  }

  if ( backupItem->dirName().isEmpty() ) {
    KMessageBox::sorry( m_widget, i18n("Selected backup is invalid.") );
    return;
  }

  mActive = true;

  logMessage( i18n("Restoring backup %1").arg( backupItem->dirName() ) );

  openKonnectors();

  mBackupDir = locateLocal( "appdata", topBackupDir() + backupItem->dirName() );

  kdDebug() << "DIRNAME: " << mBackupDir << endl;

  Konnector *k;
  for ( k = mOpenedKonnectors.first(); k; k = mOpenedKonnectors.next() ) {
    logMessage( i18n("Restoring %1.").arg( k->resourceName() ) );

    SynceeList syncees = k->syncees();

    SynceeList::ConstIterator it;
    for( it = syncees.begin(); it != syncees.end(); ++it ) {
      QString filename = backupFile( k, *it );
      kdDebug() << "FILENAME: " << filename << endl;
      QString type = (*it)->type();
      if ( (*it)->restoreBackup( filename ) ) {
        logMessage( i18n("Restored backup for %1.").arg( type ) );
      } else {
        logMessage( i18n("<b>Error:</b> Can't restore backup for %1.")
                    .arg( type ) );
      }
      k->writeSyncees();
    }
  }

  tryFinishRestore();
}

void Backup::slotSynceesWritten( Konnector *k )
{
  logMessage( i18n("Successfully written %1").arg( k->resourceName() ) );

  tryFinishRestore();
}

void Backup::slotSynceesWriteError( Konnector *k )
{
  logMessage( i18n("Error writing %1").arg( k->resourceName() ) );
  
  tryFinishRestore();
}

void Backup::tryFinishRestore()
{
  --mKonnectorCount;
  if ( mKonnectorCount > 0 ) return;
  
  logMessage( i18n("Restore finished.") );

  mActive = false;
}

void Backup::openKonnectors()
{
  mOpenedKonnectors.clear();
  mKonnectorCount = 0;

  QListViewItemIterator it( mKonnectorList );
  while ( it.current() ) {
    KonnectorCheckItem *item = static_cast<KonnectorCheckItem *>( it.current() );
    if ( item->isOn() ) {
      Konnector *k = item->konnector();
      logMessage( i18n("Connecting '%1'").arg( k->resourceName() ) );
      if ( !k->connectDevice() ) {
        logMessage( i18n("Error connecting device.") );
      } else {
        mOpenedKonnectors.append( k );
        ++mKonnectorCount;
      }
    }
    ++it;
  }
}

void Backup::deleteBackup()
{
  BackupItem *backupItem =
    static_cast<BackupItem *>( mRestoreView->currentItem() );
  
  if ( !backupItem ) {
    KMessageBox::sorry( m_widget, i18n("No backup selected.") );
    return;
  }

  int result = KMessageBox::questionYesNo( m_widget,
      i18n("Permanently delete backup '%1'?").arg( backupItem->text( 0 ) ) );
  if ( result == KMessageBox::No ) return;

  QString dirName = locateLocal( "appdata", topBackupDir() );
  dirName += backupItem->dirName();

  KProcess proc;
  proc << "rm" << "-r" << dirName;
  proc.start( KProcess::Block );

  delete backupItem;
  
  logMessage( i18n("Backup '%1' deleted").arg( dirName ) );
}

#include "backup.moc"

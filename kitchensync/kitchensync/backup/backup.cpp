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
#include <kstandarddirs.h>

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
    BackupItem( QListView *parent, const QString &filename )
      : QListViewItem( parent )
    {
      QDateTime dt = QDateTime::fromString( filename, ISODate );
      QString txt;
      if ( dt.isValid() ) {
        txt = KGlobal::locale()->formatDateTime( dt );
        mFilename = filename;
      } else {
        txt = i18n("Invalid (\"%1\")").arg( filename );
      }
      setText( 0, txt );
    }
    
    QString filename() const { return mFilename; }
    
  private:
    QString mFilename;
};

Backup::Backup( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
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

bool Backup::partIsVisible() const
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
    connect( button, SIGNAL( clicked() ), SLOT( restore() ) );

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
  logMessage( i18n("Starting backup") );

  createBackupDir();

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

  Konnector *k;
  for ( k = mOpenedKonnectors.first(); k; k = mOpenedKonnectors.next() ) {
    logMessage( i18n("Request Syncees") );
    if ( !k->readSyncees() ) {
      logMessage( i18n("Request failed.") );
    }
  }
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

void Backup::slotSynceesRead( Konnector *k, const SynceeList &syncees )
{
  logMessage( i18n("Syncees read from '%1'").arg( k->resourceName() ) );

  if ( syncees.count() == 0 ) {
    logMessage( i18n("Syncee list is empty.") );
  } else {
    logMessage( i18n("Performing backup.") );
  
    SynceeList::ConstIterator it;
    for( it = syncees.begin(); it != syncees.end(); ++it ) {
      QString filename = mBackupDir + "/" + k->identifier() + "-" +
                         (*it)->type();
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

  mKonnectorCount--;

  if ( mKonnectorCount == 0 ) {
    logMessage( i18n("Backup finished.") );
  }
}

void Backup::restore()
{
  BackupItem *backupItem =
    static_cast<BackupItem *>( mRestoreView->currentItem() );
  
  if ( !backupItem ) {
    KMessageBox::sorry( m_widget, i18n("No backup selected.") );
    return;
  }

  if ( backupItem->filename().isEmpty() ) {
    KMessageBox::sorry( m_widget, i18n("Selected backup is invalid.") );
    return;
  }

  logMessage( i18n("Restoring backup %1").arg( backupItem->filename() ) );

}

#include "backup.moc"

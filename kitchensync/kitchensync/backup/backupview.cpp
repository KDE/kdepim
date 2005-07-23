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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "backupview.h"

#include <konnector.h>
#include <syncee.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qdir.h>

using namespace KSync;

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


BackupView::BackupView( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mBackupList = new QListView( this );
  mBackupList->addColumn( i18n("Backup") );
  topLayout->addWidget( mBackupList, 1 );

  updateBackupList();

  QPushButton *button = new QPushButton( i18n("Delete"), this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( deleteBackup() ) );
}

void BackupView::updateBackupList()
{
  mBackupList->clear();

  QString dirName = locateLocal( "appdata", topBackupDir() );
  QDir dir( dirName );

  QStringList backups = dir.entryList( QDir::Dirs );
  
  QStringList::ConstIterator it;
  for( it = backups.begin(); it != backups.end(); ++it ) {
    if ( *it != "." && *it != ".." ) {
      new BackupItem( mBackupList, *it );
    }
  }
}

QString BackupView::selectedBackup()
{
  BackupItem *item = static_cast<BackupItem *>( mBackupList->selectedItem() );
  if ( !item ) return QString::null;
  
  return item->dirName();
}

QString BackupView::topBackupDir() const
{
  return "backup/";
}

void BackupView::setBackupDir( const QString &dateStr )
{
  mBackupDir = locateLocal( "appdata", topBackupDir() + dateStr + "/" );
}

void BackupView::createBackupDir()
{
  QString date = QDateTime::currentDateTime().toString( ISODate );
  mBackupDir = locateLocal( "appdata", topBackupDir() + date + "/", true );

  kdDebug() << "DIRNAME: " << mBackupDir << endl;
}

QString BackupView::backupFile( Konnector *k, Syncee *s )
{
  return mBackupDir + "/" + k->identifier() + "-" + s->type();
}

void BackupView::deleteBackup()
{
  BackupItem *backupItem =
    static_cast<BackupItem *>( mBackupList->currentItem() );
  
  if ( !backupItem ) {
    KMessageBox::sorry( this, i18n("No backup selected.") );
    return;
  }

  int result = KMessageBox::questionYesNo( this,
      i18n("Permanently delete backup '%1'?").arg( backupItem->text( 0 ) ) );
  if ( result == KMessageBox::No ) return;

  QString dirName = locateLocal( "appdata", topBackupDir() );
  dirName += backupItem->dirName();

  KProcess proc;
  proc << "rm" << "-r" << dirName;
  proc.start( KProcess::Block );

  delete backupItem;

  emit backupDeleted( dirName );
}

#include "backupview.moc"

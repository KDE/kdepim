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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "restore.h"

#include "backupview.h"

#include <konnector.h>
#include <core.h>
#include <engine.h>

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

using namespace KCal;
using namespace KSync;

typedef KParts::GenericFactory<Restore> RestoreFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_restore, RestoreFactory )

Restore::Restore( QWidget *parent, const char *name,
                  QObject *, const char *,const QStringList & )
  : ActionPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("kcmdrkonqi", KIcon::Desktop, 48 );
}

KAboutData *Restore::createAboutData()
{
  return new KAboutData("KSyncRestore", I18N_NOOP("Sync Restore Part"), "0.0" );
}

Restore::~Restore()
{
  delete m_widget;
}

QString Restore::type() const
{
  return QString::fromLatin1("Restore");
}

QString Restore::title() const
{
  return i18n("Konnector Restore");
}

QString Restore::description() const
{
  return i18n("Restore for Konnectors");
}

QPixmap *Restore::pixmap()
{
  return &m_pixmap;
}

QString Restore::iconName() const
{
  return QString::fromLatin1("kcmsystem");
}

bool Restore::hasGui() const
{
  return true;
}

QWidget *Restore::widget()
{
  if( !m_widget ) {
    m_widget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( m_widget );
    topLayout->setSpacing( KDialog::spacingHint() );
    
    QBoxLayout *konnectorLayout = new QHBoxLayout( topLayout );

    QBoxLayout *restoreLayout = new QVBoxLayout( konnectorLayout );

    mBackupView = new BackupView( m_widget );
    restoreLayout->addWidget( mBackupView );

    mBackupView->updateBackupList();

    mLogView = new QTextView( m_widget );
    mLogView->setTextFormat( LogText );
    topLayout->addWidget( mLogView );

    logMessage( i18n("Ready.") );
  }
  return m_widget;
}

void Restore::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  kdDebug() << "LOG: " << text << endl;

  mLogView->append( text );
}

void Restore::executeAction()
{
  logMessage( i18n("Starting Restore") );

  QString backup = mBackupView->selectedBackup();
  
  if ( backup.isNull() ) {
    KMessageBox::sorry( m_widget, i18n("No backup selected.") );
    return;
  }

  if ( backup.isEmpty() ) {
    KMessageBox::sorry( m_widget, i18n("Selected backup is invalid.") );
    return;
  }

  logMessage( i18n("Restoring backup %1").arg( backup ) );

  mBackupView->setBackupDir( backup );

  Konnector::List konnectors = core()->engine()->konnectors();
  Konnector *k;
  for( k = konnectors.first(); k; k = konnectors.next() ) {
    restoreKonnector( k );
  }

  logMessage( i18n("Restore finished.") );

  mBackupView->updateBackupList();
}

void Restore::restoreKonnector( Konnector *k )
{
  logMessage( i18n("Restoring %1.").arg( k->resourceName() ) );

  SynceeList syncees = k->syncees();

  SynceeList::ConstIterator it;
  for( it = syncees.begin(); it != syncees.end(); ++it ) {
    if ( !(*it)->isValid() ) continue;

    QString filename = mBackupView->backupFile( k, *it );
    kdDebug() << "FILENAME: " << filename << endl;
    QString type = (*it)->type();
    if ( (*it)->restoreBackup( filename ) ) {
      logMessage( i18n("Restored backup for %1.").arg( type ) );
    } else {
      logMessage( i18n("<b>Error:</b> Can't restore backup for %1.")
                  .arg( type ) );
    }
  }
}

#include "restore.moc"

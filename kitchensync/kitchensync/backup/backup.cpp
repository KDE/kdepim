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

    QFrame *restoreFrame = new QFrame( m_widget );
    restoreFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    restoreLayout->addWidget( restoreFrame, 1 );

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

void Backup::logMessage( const QString &message )
{
  QString text = "<b>" + QTime::currentTime().toString() + "</b>: ";
  text += message;

  kdDebug() << "LOG: " << text << endl;

  mLogView->append( text );
}

void Backup::actionSync()
{
  logMessage( i18n("actionSync()") );
}

void Backup::restore()
{
  KMessageBox::information( m_widget, i18n("Restore isn't implemented yet") );
}

#include "backup.moc"

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

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlayout.h>

typedef KParts::GenericFactory< KSync::Debugger> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_debugger, DebuggerFactory );

using namespace KSync ;

Debugger::Debugger( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ManipulatorPart( parent, name ), m_widget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("package_settings", KIcon::Desktop, 48 );
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

    commandLayout->addStretch();


    mLogView = new QTextView( m_widget );
    topLayout->addWidget( mLogView );    
  }
  return m_widget;
}

void Debugger::configureKonnector()
{
  Konnector *k = currentKonnector();
  if ( !k ) {
    KMessageBox::sorry( m_widget, i18n("Konnector isn't loaded" ) );
  } else {
    ConfigWidget *configWidget = k->configWidget( m_widget );
    configWidget->show();
  }
}

Konnector *Debugger::currentKonnector()
{
  QString konnectorName = mKonnectorCombo->currentText();
  
  KonnectorProfile::ValueList konnectors =
      core()->konnectorProfileManager()->list();

  KonnectorProfile::ValueList::ConstIterator it;
  for( it = konnectors.begin(); it != konnectors.end(); ++it ) {
    if ( konnectorName == (*it).name() ) break;
  }

  if ( it == konnectors.end() ) return 0;

  return (*it).konnector();
}

#include "debugger.moc"

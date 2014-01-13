/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "debugwidget.h"

#include "tracernotificationinterface.h"
#include "connectionpage.h"

#include <akonadi/control.h>

#include <KFileDialog>
#include <KLocale>
#include <KTabWidget>
#include <KTextEdit>
#include <Akonadi/ServerManager>

#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QCheckBox>

using org::freedesktop::Akonadi::DebugInterface;

DebugWidget::DebugWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QString service = "org.freedesktop.Akonadi";
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += "." + Akonadi::ServerManager::instanceIdentifier();
  }
  mDebugInterface = new DebugInterface( service, "/debug", QDBusConnection::sessionBus(), this );
  QCheckBox *cb = new QCheckBox( i18n("Enable debugger"), this );
  cb->setChecked( mDebugInterface->isValid() && mDebugInterface->tracer().value() == QLatin1String( "dbus" ) );
  connect( cb, SIGNAL(toggled(bool)), SLOT(enableDebugger(bool)) );
  layout->addWidget( cb );

  QSplitter *splitter = new QSplitter( Qt::Vertical, this );
  splitter->setObjectName( "debugSplitter" );
  layout->addWidget( splitter );

  mConnectionPages = new KTabWidget( splitter );
  mConnectionPages->setTabsClosable( true );

  mGeneralView = new KTextEdit( splitter );
  mGeneralView->setReadOnly( true );

  ConnectionPage *page = new ConnectionPage( "All" );
  page->showAllConnections( true );
  mConnectionPages->addTab( page, "All" );

  connect( mConnectionPages, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)) );

  org::freedesktop::Akonadi::TracerNotification *iface = new org::freedesktop::Akonadi::TracerNotification( QString(), "/tracing/notifications", QDBusConnection::sessionBus(), this );

  connect( iface, SIGNAL(connectionStarted(QString,QString)),
           this, SLOT(connectionStarted(QString,QString)) );
  connect( iface, SIGNAL(connectionEnded(QString,QString)),
           this, SLOT(connectionEnded(QString,QString)) );
  connect( iface, SIGNAL(signalEmitted(QString,QString)),
           this, SLOT(signalEmitted(QString,QString)) );
  connect( iface, SIGNAL(warningEmitted(QString,QString)),
           this, SLOT(warningEmitted(QString,QString)) );
  connect( iface, SIGNAL(errorEmitted(QString,QString)),
           this, SLOT(errorEmitted(QString,QString)) );

  // in case we started listening when the connection is already ongoing
  connect( iface, SIGNAL(connectionDataInput(QString,QString)),
           this, SLOT(connectionStarted(QString,QString)) );
  connect( iface, SIGNAL(connectionDataOutput(QString,QString)),
           this, SLOT(connectionStarted(QString,QString)) );

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  layout->addLayout( buttonLayout );

  QPushButton *clearAllButton = new QPushButton( "Clear All Tabs", this );
  QPushButton *clearCurrentButton = new QPushButton( "Clear Current Tab", this );
  QPushButton *clearGeneralButton = new QPushButton( "Clear Information View", this );
  QPushButton *closeAllTabsButton = new QPushButton( "Close All Tabs", this );
  QPushButton *saveRichtextButton = new QPushButton( "Save as RichText...", this );

  buttonLayout->addWidget( clearAllButton );
  buttonLayout->addWidget( clearCurrentButton );
  buttonLayout->addWidget( clearGeneralButton );
  buttonLayout->addWidget( closeAllTabsButton );
  buttonLayout->addWidget( saveRichtextButton );

  connect( clearAllButton, SIGNAL(clicked()), this, SLOT(clearAllTabs()) );
  connect( clearCurrentButton, SIGNAL(clicked()), this, SLOT(clearCurrentTab()) );
  connect( clearGeneralButton, SIGNAL(clicked()), mGeneralView, SLOT(clear()) );
  connect( closeAllTabsButton, SIGNAL(clicked()), this, SLOT(closeAllTabs()) );
  connect( saveRichtextButton, SIGNAL(clicked()), this, SLOT(saveRichText()) );

  Akonadi::Control::widgetNeedsAkonadi( this );
}

void DebugWidget::connectionStarted( const QString &identifier, const QString &msg )
{
  Q_UNUSED( msg );
  if ( mPageHash.contains( identifier ) )
    return;

  ConnectionPage *page = new ConnectionPage( identifier );
  mConnectionPages->addTab( page, identifier );

  mPageHash.insert( identifier, page );
}

void DebugWidget::connectionEnded( const QString &identifier, const QString& )
{
  if ( !mPageHash.contains( identifier ) )
    return;

  QWidget *widget = mPageHash[ identifier ];

  mConnectionPages->removeTab( mConnectionPages->indexOf( widget ) );

  mPageHash.remove( identifier );
  delete widget;
}

void DebugWidget::tabCloseRequested( int index )
{
  if ( index != 0 ) {
    QWidget *page = mConnectionPages->widget( index );
    QMutableHashIterator<QString, ConnectionPage*> it( mPageHash );
    while ( it.hasNext() ) {
      it.next();
      if ( it.value() == page ) {
        it.remove();
        break;
      }
    }

    mConnectionPages->removeTab( index );
    delete page;
  }
}

void DebugWidget::clearAllTabs()
{
  ConnectionPage *page = qobject_cast<ConnectionPage*>( mConnectionPages->widget( 0 ) );
  if ( page )
    page->clear();

  QMutableHashIterator<QString, ConnectionPage*> it( mPageHash );
  while ( it.hasNext() )
    it.next().value()->clear();
}

void DebugWidget::clearCurrentTab()
{
  ConnectionPage *page = qobject_cast<ConnectionPage*>( mConnectionPages->currentWidget() );
  if ( !page )
    return;

  page->clear();
}

void DebugWidget::closeAllTabs()
{
  ConnectionPage *page = qobject_cast<ConnectionPage*>( mConnectionPages->widget( 0 ) );
  if ( page ) {
    page->clear();
  }

  while ( mConnectionPages->count() > 1 ) {
    mConnectionPages->removeTab( 1 );
  }
  qDeleteAll(mPageHash);
  mPageHash.clear();
}

void DebugWidget::signalEmitted( const QString &signalName, const QString &msg )
{
  mGeneralView->append( QString( "<font color=\"green\">%1 ( %2 )</font>" ).arg( signalName, msg ) );
}

void DebugWidget::warningEmitted( const QString &componentName, const QString &msg )
{
  mGeneralView->append( QString( "<font color=\"blue\">%1: %2</font>" ).arg( componentName, msg ) );
}

void DebugWidget::errorEmitted( const QString &componentName, const QString &msg )
{
  mGeneralView->append( QString( "<font color=\"red\">%1: %2</font>" ).arg( componentName, msg ) );
}

void DebugWidget::enableDebugger(bool enable)
{
  mDebugInterface->setTracer( enable ? QLatin1String( "dbus" ) : QLatin1String( "null" ) );
}

void DebugWidget::saveRichText()
{
  ConnectionPage *page = qobject_cast<ConnectionPage*>( mConnectionPages->currentWidget() );
  if ( !page )
    return;

  const QString fileName = KFileDialog::getSaveFileName();
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return;

  file.write( page->toHtml().toUtf8() );
  file.close();
}

#include "debugwidget.moc"

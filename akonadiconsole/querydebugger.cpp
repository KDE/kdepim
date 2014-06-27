/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "querydebugger.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QMenu>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>
#include <boost/concept_check.hpp>
#include <QtCore/QDateTime>

#include <AkonadiCore/servermanager.h>
#include <AkonadiCore/control.h>

#include <KTextEdit>
#include <KGlobalSettings>
#include <KLocalizedString>
#include <KDebug>
#include <QFontDatabase>

Q_DECLARE_METATYPE(QList< QList<QVariant> >)

QueryDebugger::QueryDebugger( QWidget* parent ):
  QWidget( parent )
{
  qDBusRegisterMetaType< QList< QList<QVariant> > >();

  QString service = QLatin1String( "org.freedesktop.Akonadi" );
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += "." + Akonadi::ServerManager::instanceIdentifier();
  }
  mDebugger = new org::freedesktop::Akonadi::StorageDebugger( service,
                QLatin1String( "/storageDebug" ), QDBusConnection::sessionBus(), this );

  connect( mDebugger, SIGNAL(queryExecuted(double,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString)),
           this, SLOT(addQuery(double,uint,QString,QMap<QString,QVariant>,int,QList<QList<QVariant> >,QString)) );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QCheckBox* enableCB = new QCheckBox( this );
  enableCB->setText( "Enable query debugger (slows down server!)");
  enableCB->setChecked( mDebugger->isSQLDebuggingEnabled() );
  connect( enableCB, SIGNAL(toggled(bool)), mDebugger, SLOT(enableSQLDebugging(bool)) );
  layout->addWidget(enableCB);

  mView = new KTextEdit( this );
  mView->setReadOnly( true );
  mView->setContextMenuPolicy( Qt::CustomContextMenu );
  mView->setFont( QFontDatabase::systemFont(QFontDatabase::FixedFont) );
  layout->addWidget( mView );

  connect( mView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenu(QPoint)) );

  Akonadi::Control::widgetNeedsAkonadi( this );
}

QueryDebugger::~QueryDebugger()
{
  // Disable debugging when turning off Akonadi Console so that we don't waste
  // resources on server
  mDebugger->enableSQLDebugging( false );
}

void QueryDebugger::contextMenu( const QPoint &pos )
{
  QMenu menu;
  menu.addAction( i18n( "Clear View" ), mView, SLOT(clear()) );
  menu.exec( mapToGlobal( pos ) );
}

void QueryDebugger::addQuery( double sequence, uint duration, const QString &query,
                              const QMap<QString, QVariant> &values,
                              int resultsCount, const QList<QList<QVariant> > &result,
                              const QString &error )
{
  QString q = query;
  const QStringList keys = values.uniqueKeys();
  Q_FOREACH ( const QString &key, keys ) {
    int pos = q.indexOf( QLatin1String( "?" ) );
    const QVariant val = values.value( key );
    q.replace( pos, 1, variantToString( val ) );
  }

  mView->append( QString::fromLatin1( "%1: <font color=\"blue\">%2</font>") .arg( sequence ).arg( q ) );

  if ( !error.isEmpty() ) {
    mView->append( QString::fromLatin1( "<font color=\"red\">Error: %1</font>\n").arg( error ) );
    return;
  }

  mView->append( QString::fromLatin1( "<font color=\"green\">Success</font>: Query took %1 msecs ").arg( duration ) );
  if ( query.startsWith( QLatin1String( "SELECT") ) ) {
    mView->append( QString::fromLatin1( "Fetched %1 results").arg( resultsCount ) );
  } else {
    mView->append( QString::fromLatin1( "Affected %1 rows").arg( resultsCount ) );
  }

  if ( !result.isEmpty() ) {
    const QVariantList headerRow = result.first();
    QString header;
    for ( int i = 0; i < headerRow.size(); ++i ) {
      if ( i > 0 ) {
        header += QLatin1String( " | " );
      }
      header += headerRow.at( i ).toString();
    }
    mView->append( header );

    QString sep;
    mView->append( sep.fill( QLatin1Char('-'), header.length() ) );

    for ( int row = 1; row < result.count(); ++row ) {
      const QVariantList columns = result.at( row );
      QString rowStr;
      for ( int column = 0; column < columns.count(); ++column ) {
        if ( column > 0 ) {
          rowStr += QLatin1String( " | " );
        }
        rowStr += variantToString( columns.at( column ) );
      }
      mView->append( rowStr );
    }
  }

  mView->append( QLatin1String( "\n" ) );
}

QString QueryDebugger::variantToString( const QVariant &val )
{
  if ( val.canConvert( QVariant::String ) ) {
    return val.toString();
  } else if ( val.canConvert( QVariant::QVariant::DateTime ) ) {
    return val.toDateTime().toString( Qt::ISODate );
  }

  QDBusArgument arg = val.value<QDBusArgument>();
  if ( arg.currentType() == QDBusArgument::StructureType ) {
    QDateTime t = qdbus_cast<QDateTime>( arg );
    if ( t.isValid() ) {
      return t.toString( Qt::ISODate );
    }
  }

  return QString();
}




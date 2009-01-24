/*
    Copyright (c) 20079 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "dbbrowser.h"

#include <akonadi/private/xdgbasedirs_p.h>

#include <KMessageBox>

#include <QSettings>
#include <QSqlError>
#include <QSqlTableModel>

using namespace Akonadi;

DbBrowser::DbBrowser(QWidget* parent) :
  QWidget( parent ),
  mTableModel( 0 )
{
  ui.setupUi( this );

  const QString serverConfigFile = XdgBaseDirs::akonadiServerConfigFile( XdgBaseDirs::ReadWrite );
  QSettings settings( serverConfigFile, QSettings::IniFormat );

  const QString driver = settings.value( "General/Driver", "QMYSQL" ).toString();
  mDatabase = QSqlDatabase::addDatabase( driver );
  settings.beginGroup( driver );
  mDatabase.setHostName( settings.value( "Host", QString() ).toString() );
  mDatabase.setDatabaseName( settings.value( "Name", "akonadi" ).toString() );
  mDatabase.setUserName( settings.value( "User", QString() ).toString() );
  mDatabase.setPassword( settings.value( "Password", QString() ).toString() );
  mDatabase.setConnectOptions( settings.value( "Options", QString() ).toString() );
  if ( !mDatabase.open() ) {
    KMessageBox::error( this, i18n( "Failed to connect to database: %1", mDatabase.lastError().text() ) );
  }

  ui.tableBox->addItems( mDatabase.tables(QSql::AllTables) );

  ui.refreshButton->setIcon( KIcon( "view-refresh" ) );
  connect( ui.refreshButton, SIGNAL(clicked()), SLOT(refreshClicked()) );
}

void DbBrowser::refreshClicked()
{
  const QString table = ui.tableBox->currentText();
  if ( table.isEmpty() )
    return;
  delete mTableModel;
  mTableModel = new QSqlTableModel( this, mDatabase );
  mTableModel->setTable( table );
  mTableModel->setEditStrategy( QSqlTableModel::OnRowChange );
  mTableModel->select();
  ui.tableView->setModel( mTableModel );
}

#include "dbbrowser.moc"

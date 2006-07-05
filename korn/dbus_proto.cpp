/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "dbus_proto.h"

#include "account_input.h"
#include "dbusdrop.h"

#include <kconfigbase.h>
#include <klocale.h>

#include <QMap>
#include <q3ptrlist.h>
#include <QVector>

KMailDrop* DBUS_Protocol::createMaildrop( KConfigGroup* ) const
{
	return new DBUSDrop();
}

QMap< QString, QString > * DBUS_Protocol::createConfig( KConfigGroup* config, const QString& ) const
{
	QMap< QString, QString > *result = new QMap< QString, QString >;

	result->insert( "dbusname", config->readEntry( "dbusname", "korn_dbus" ) );

	return result;
}

QString DBUS_Protocol::configName() const
{
	return "dbus";
}

void DBUS_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "DBUS" );
}

void DBUS_Protocol::configFields( QVector< QWidget* >* vector, const QObject*, QList< AccountInput* >* result ) const
{
	result->append( new TextInput( vector->at( 0 ), i18n( "DBUS name" ), TextInput::text, "korn_dbus", "dbusname" ) );
}

void DBUS_Protocol::readEntries( QMap< QString, QString >* ) const
{
}

void DBUS_Protocol::writeEntries( QMap< QString, QString >* ) const
{
}

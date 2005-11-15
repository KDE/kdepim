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


#include "process_proto.h"

#include "account_input.h"
#include "process_drop.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <qlist.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvector.h>

const Protocol* Process_Protocol::getProtocol( KConfigGroup* ) const
{
	return this;
}

KMailDrop* Process_Protocol::createMaildrop( KConfigGroup* ) const
{
	return new ProcessDrop();
}

QMap< QString, QString > * Process_Protocol::createConfig( KConfigGroup* config, const QString& ) const
{
	QMap< QString, QString > *result = new QMap< QString, QString >();
	
	result->insert( "program", config->readEntry( "program", "" ) );

	return result;
}

void Process_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "Process" );
}

void Process_Protocol::configFields( QVector< QWidget* >* vector, const QObject*, QList< AccountInput* > *result ) const
{
	result->append( new URLInput( vector->at( 0 ), i18n( "Program:" ), "", "program" ) );
}

void Process_Protocol::readEntries( QMap< QString, QString >* ) const
{
}

void Process_Protocol::writeEntries( QMap< QString, QString >* ) const
{
}


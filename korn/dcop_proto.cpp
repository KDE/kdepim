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


#include "dcop_proto.h"

#include "account_input.h"
#include "dcopdrop.h"

#include <kconfigbase.h>
#include <klocale.h>

#include <qmap.h>
#include <qptrlist.h>
#include <qptrvector.h>

KMailDrop* DCOP_Protocol::createMaildrop( KConfigGroup* ) const
{
	return new DCOPDrop();
}

QMap< QString, QString > * DCOP_Protocol::createConfig( KConfigGroup* config, const QString& ) const
{
	QMap< QString, QString > *result = new QMap< QString, QString >;

	result->insert( "dcopname", config->readEntry( "dcopname", "korn_dcop" ) );

	return result;
}

void DCOP_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "DCOP" );
}

void DCOP_Protocol::configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput >* result ) const
{
	result->append( new TextInput( (QWidget*)vector->at( 0 ), i18n( "DCOP name" ), TextInput::text, "korn_dcop", "dcopname" ) );
}

void DCOP_Protocol::readEntries( QMap< QString, QString >* ) const
{
}

void DCOP_Protocol::writeEntries( QMap< QString, QString >* ) const
{
}

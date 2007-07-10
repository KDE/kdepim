/* cudcounter.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "cudcounter.h"

#include <klocalizedstring.h>

CUDCounter::CUDCounter()
{
}

void CUDCounter::setStartCount( unsigned int t )
{
	fStart = t;
}

void CUDCounter::setEndCount( unsigned int t )
{
	fEnd = t;
}

void CUDCounter::created( unsigned int c )
{
	fC += c;
}

void CUDCounter::updated( unsigned int u )
{
	fU += u;
}

void CUDCounter::deleted( unsigned int d )
{
	fD += d;
}

int CUDCounter::volatilityCount() const
{
	return fC + fU + fD;
}

int CUDCounter::volatilityPercent() const
{
	return (fStart > 0 ? volatilityCount() / fStart : 100 );
}

QString CUDCounter::moo() const
{
	QString result = i18n("Start: %1. End: %2. ",fStart,fEnd);

	if (fC > 0) result += i18n("%1 new. ",fC);
	if (fU > 0) result += i18n("%1 changed. ",fU);
	if (fD > 0) result += i18n("%1 deleted. ",fD);

	if ( (fC+fU+fD) <= 0) result += i18n("No changes made. ");

	return result;
}

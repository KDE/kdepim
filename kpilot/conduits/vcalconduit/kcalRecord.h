#ifndef _KPILOT_KCALRECORD_H
#define _KPILOT_KCALRECORD_H
/*
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

class PilotRecordBase;

namespace KCal
{
	class Incidence;
}

namespace KCalSync
{
	void setCategory(PilotRecordBase *de,
		const KCal::Incidence *incidence,
		const CategoryAppInfo &info);
	void setCategory(KCal::Incidence *e,
		const PilotRecordBase *de,
		const CategoryAppInfo &info);
}

#endif


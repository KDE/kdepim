#ifndef _KPILOT_PILOTCARD_H
#define _KPILOT_PILOTCARD_H
/* pilotCard.h			KPilot
**
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This class is a wrapper around pilot-link's CardInfo structure
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _PILOT_DLP_H_
#include <pi-dlp.h>
#endif

class KPilotCard
{
public:
	KPilotCard() { ::memset(&fCard,0,sizeof(struct CardInfo)); }
	KPilotCard(const CardInfo* card) { fCard = *card; }

	CardInfo *cardInfo() { return &fCard; }

	/**
	* Ensures the names are properly terminated.  Needed incase we
	* are syncing a new and bogus pilot.
	*/
	void boundsCheck() {}

	const int getCardIndex() const          { return fCard.card; }
	const int getCardVersion() const        { return fCard.version; }
	unsigned long getRomSize() const        { return fCard.romSize; }
	unsigned long getRamSize() const        { return fCard.ramSize; }
	unsigned long getRamFree() const        { return fCard.ramFree; }
	const char* getCardName() const         { return fCard.name; }
	const char* getCardManufacturer() const { return fCard.manufacturer; }

private:
	struct CardInfo fCard;
};

#endif

/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
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

#include "sortedmailsubject.h"

int SortedMailSubject::compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
{
	int date1, date2;
	date1 = ( (KornMailSubject*)item1 )->getDate();
	date2 = ( (KornMailSubject*)item2 )->getDate();
	
	if( date1 < date2 )
		return -1;
	else if( date1 == date2 )
		return 0;
	else
		return 1;
}

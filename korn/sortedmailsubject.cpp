//Copyright (C) 2004, Mart Kelder

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

/***************************************************************************
                          knuserentry.cpp  -  description
                             -------------------

    copyright            : (C) 1999 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "knuserentry.h"


KNUserEntry::KNUserEntry()
{
}



KNUserEntry::~KNUserEntry()
{
}



void KNUserEntry::load(KConfigBase *c)
{
	n_ame=c->readEntry("Name").local8Bit();
	e_mail=c->readEntry("Email").local8Bit();
	r_eplyTo=c->readEntry("Reply-To").local8Bit();
	o_rga=c->readEntry("Org").local8Bit();
	s_igPath=c->readEntry("sigFile").local8Bit();
}



void KNUserEntry::save(KConfigBase *c)
{
	c->writeEntry("Name", n_ame.data());
	c->writeEntry("Email", e_mail.data());
	c->writeEntry("Reply-To", r_eplyTo.data());
	c->writeEntry("Org", o_rga.data());
	c->writeEntry("sigFile", s_igPath.data());
}



bool KNUserEntry::isEmpty()
{
	return (	n_ame.isEmpty() && 	e_mail.isEmpty() &&
						r_eplyTo.isEmpty()	 && o_rga.isEmpty() &&
						s_igPath.isEmpty() );
}

/***************************************************************************
                          knserverinfo.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
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


#include "knserverinfo.h"

KNServerInfo::KNServerInfo()
{
    t_ype=STnntp;
    p_ort=119;
    t_imeout=60;
 	h_old=300;
	i_d=-1;
}



KNServerInfo::~KNServerInfo()
{
}



void KNServerInfo::copy(KNServerInfo *i)
{
	t_ype=i->type();
    p_ort=i->port();
 	h_old=i->hold();
 	t_imeout=i->timeout();
	s_erver=i->server().copy();
	u_ser=i->user().copy();
	p_ass=i->pass().copy();
}



void KNServerInfo::clear()
{
	t_ype=STnntp;
    p_ort=119;
    t_imeout=60;
    h_old=300;
	s_erver = "";
	u_ser = "";
	p_ass = "";
}



bool KNServerInfo::isEqual(KNServerInfo *i)
{
	return (	(t_ype==i->type()) 	&&
						(s_erver==i->server()) &&
        		(p_ort==i->port()) &&
 						(h_old==i->hold()) &&
 						(t_imeout==i->timeout()) &&
						(u_ser==i->user()) &&
						(p_ass==i->pass())						);
}

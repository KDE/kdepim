/***************************************************************************
                          kncontrolarticle.cpp  -  description
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


#include "kncontrolarticle.h"
#include "knhdrviewitem.h"
#include "utilities.h"

#include <kiconloader.h>

KNControlArticle::KNControlArticle() : KNSavedArticle(AStoPost)
{
	c_tlType=CTcancel;
}



KNControlArticle::~KNControlArticle()
{
}



void KNControlArticle::updateListItem()
{
	if(!i_tem) return;
	i_tem->setText(1, d_estination);
	i_tem->setText(0, s_ubject);
	i_tem->setText(3, timeString());
	i_tem->setPixmap(0, UserIcon("ctlart"));
}

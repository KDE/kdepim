/***************************************************************************
                          knkeysettings.cpp  -  description
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

#include <klocale.h>
#include <kkeydialog.h>

#include "knkeysettings.h"
#include "knglobals.h"

KNKeySettings::KNKeySettings(QWidget *parent) : KNSettingsWidget(parent)
{
	keys=xTop->accel()->keyDict();
	kc=new KKeyChooser(&keys, this);
	
	stdBtn=new QPushButton(i18n("Reset"), this);
	stdBtn->setFixedSize(stdBtn->sizeHint());
	connect(stdBtn, SIGNAL(clicked()), kc, SLOT(allDefault()));
}



KNKeySettings::~KNKeySettings()
{
}



void KNKeySettings::resizeEvent(QResizeEvent *)
{
	kc->setGeometry(10,10, width()-20, height()-stdBtn->height()-35);
	stdBtn->move(10, kc->height()+25);	
}



void KNKeySettings::apply()
{
	kc->listSync();
}


/***************************************************************************
                          knusersettings.cpp  -  description
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
#include <kglobal.h>
#include <kconfig.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include "knappmanager.h"
#include "knglobals.h"
#include "knuserwidget.h"
#include "knuserentry.h"
#include "knusersettings.h"


KNUserSettings::KNUserSettings(QWidget *p) : KNSettingsWidget(p)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  uw=new KNUserWidget(this);
  layout->addWidget(uw);
  init();

  layout->setResizeMode(QLayout::Minimum);
}



KNUserSettings::~KNUserSettings()
{
}



void KNUserSettings::init()
{
  uw->setData(knGlobals.appManager->defaultUser());
}



void KNUserSettings::apply()
{
  uw->applyData();
}

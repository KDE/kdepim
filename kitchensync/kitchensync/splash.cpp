/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

#include "splash.h"

using namespace KSync;

Splash::Splash()
  : QWidget( 0, "splash", WStyle_Splash | WStyle_Customize | WDestructiveClose )
{
  QPixmap splash;
  splash.load(locate("appdata", "pics/startlogo.png") );
  setBackgroundPixmap(splash  );

  QRect desk = KGlobalSettings::splashScreenDesktopGeometry();
  setGeometry(desk.center().x()-splash.width()/2,
	      desk.center().y()-splash.height()/2,
	      splash.width(),
	      splash.height() );
  show(); // just to be sure
}

Splash::~Splash()
{

}

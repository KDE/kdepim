/*
† † † †This file is part of the OPIE Project
† † † †Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† †                   2002 Maximilian Reiﬂ <harlekin@handhelds.org>
† † † † † †

† † † † † † † †=.            
† † † † † † †.=l.            
† † † † † †.>+-=
†_;:, † † .> † †:=|.         This program is free software; you can 
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.        
† † .i_,=:_. † † †-<s.       This program is distributed in the hope that  
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-        
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB. 
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/



#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

#include "ksync_splash.h"

using namespace KSync;

Splash::Splash()
  : QWidget( 0, "splash", WStyle_NoBorderEx | WStyle_Customize | WDestructiveClose )
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

};

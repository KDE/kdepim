/*
    This file is part of KitchenSync.
    
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"

using namespace KSync;

AboutData *AboutData::mSelf = 0;

AboutData::AboutData()
  : KAboutData( "kitchensync", I18N_NOOP("KitchenSync"),
                "0.1",
                I18N_NOOP("Synchronize Data with KDE"),
                KAboutData::License_GPL,
                "(c) 2001-2002 Holger Freyther\n"
                "(c) 2002 Maximilian Reiss\n"
                "(c) 2003 Cornelius Schumacher",
                0,
                "http://pim.kde.org" )
{
  addAuthor( "Cornelius Schumacher", "", "schumacher@kde.org" );
  addAuthor( "Maximilian Reiss", I18N_NOOP("Current Maintainer"),
             "harlekin@handhelds.org" );
  addAuthor( "Holger Freyther", I18N_NOOP("Current Maintainer"),
             "zecke@handhelds.org" );
  addCredit( "Alexandra Chalupka",
             I18N_NOOP("For her understanding that I'm an addict."), 0 );
  addCredit( "HP ( former Compaq )",
             I18N_NOOP("For all the support HP is giving OpenSource projects "
                       "at handhelds.org. Thanks a lot."), 0 );
  addCredit( "Bipolar and the rest of the Opie TEAM!",
             I18N_NOOP("Testing, testing, testing" ),
             "opie@handhelds.org" );
  addCredit( "Philib Bundell",
             I18N_NOOP("For being such a nice guy." ),
             "pb@gnu.org" );
}

AboutData *AboutData::self()
{
  if ( !mSelf ) mSelf = new AboutData;
  return mSelf;
}

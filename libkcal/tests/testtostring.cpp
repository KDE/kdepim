/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <iostream>
#include <kdebug.h>

#include "event.h"
#include "icalformat.h"

using namespace KCal;

int main( int, char ** )
{

 // std::cout << "Hello World!" << std::endl;
  Event *ev = new Event;
  ev->setSummary("Griazi");
  ICalFormat iformat;
  QString icalstr = iformat.toICalString(ev);
  kdDebug(5800) << icalstr << endl;
  Incidence *ev2 = iformat.fromString(icalstr);
  kdDebug(5800) << "Event reread!" << endl ;
  
  if (ev2)
    kdDebug(5800) << iformat.toICalString(ev2) << endl;
  else
    kdDebug(5800) << "Could not read incidence" << endl;
}

/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "vcaldrag.h"
#include "vcc.h"
#include "vobject.h"

using namespace KCal;

VCalDrag::VCalDrag(VObject *vcal, QWidget *parent, const char *name)
  : QStoredDrag("text/x-vCalendar", parent, name)
{
  char *buf = writeMemVObject(0, 0, vcal);

  QByteArray data;

  data.assign(buf, strlen(buf));
  
  setEncodedData(data);
  // we don't delete the buf because QByteArray claims it will handle that?!?
}

bool VCalDrag::canDecode(QMimeSource *me)
{
  return me->provides("text/x-vCalendar");  
}

bool VCalDrag::decode(QMimeSource *de, VObject **vcal)
{
  QByteArray payload = de->encodedData("text/x-vCalendar");
  if (payload.size()) { // check to see if we got this kind of data
    *vcal = Parse_MIME(payload.data(), payload.size());
    if (*vcal) { // only return true if there was no parse error.
      return TRUE;
    }
  }
  return FALSE;
}


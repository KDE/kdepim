/*
    knglobals.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "knglobals.h"


#include <kconfig.h>
#include <kstaticdeleter.h>

KConfig *KNGlobals::c_onfig = 0;
static KStaticDeleter<KConfig> c_onfigSD;

KConfig* KNGlobals::config()
{
  if (!c_onfig)
    c_onfigSD.setObject(c_onfig, new KConfig( "knoderc"));
  return c_onfig;
}

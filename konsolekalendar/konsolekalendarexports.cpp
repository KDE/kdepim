/***************************************************************************
        konsolekalendaradd.cpp  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2002 by Tuukka Pasanen
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include "calendarlocal.h"
#include "calendar.h"
#include "event.h"

#include "konsolekalendarexports.h"

using namespace KCal;
using namespace std;

KonsoleKalendarExports::KonsoleKalendarExports( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
}

KonsoleKalendarExports::~KonsoleKalendarExports()
{
}

void KonsoleKalendarExports::exportAsHTML(){
}

void KonsoleKalendarExports::exportAsTxt(){
}

void KonsoleKalendarExports::exportAsTxtKorganizer(){
}

void KonsoleKalendarExports::exportAsCSV(){
}



#ifndef _KONSOLEKALENDARCHANGE_H_
#define _KONSOLEKALENDARCHANGE_H_

/***************************************************************************
        konsolekaledarchange.h  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
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

#include <qdatetime.h>
#include <qstring.h>

#include "konsolekalendarvariables.h"

namespace KCal
{

class KonsoleKalendarChange
{
  public:
    KonsoleKalendarChange( KonsoleKalendarVariables *variables );
    ~KonsoleKalendarChange();

   /**
    * Changes some event
    */

    bool changeEvent();

 private:

   /**
    * Variables are here
    */

     KonsoleKalendarVariables *m_variables;
};

}
#endif

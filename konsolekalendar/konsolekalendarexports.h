#ifndef _KONSOLEKALENDAREXPORTS_H_
#define _KONSOLEKALENDAREXPORTS_H_

/***************************************************************************
        konsolekaledarexports.h  -  description
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

#include <qtextstream.h>
#include "konsolekalendarvariables.h"

namespace KCal
{

class KonsoleKalendarExports
{
public:
  KonsoleKalendarExports( KonsoleKalendarVariables *variables  = 0);
  ~KonsoleKalendarExports();

  /*
   * These can be changed at anytime;)
   * without a notice
   */
   

  bool exportAsTxt( QTextStream *ts, Event *event );
  bool exportAsCSV( QTextStream *ts, Event *event );
   

private:
   KonsoleKalendarVariables *m_variables;
   QDate m_lastDate;
   bool m_firstEntry;
	   
};

}
#endif

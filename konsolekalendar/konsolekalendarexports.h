/*******************************************************************************
 * konsolekalendarexports.h                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2004  Allen Winter <winter@kde.org>                      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDAREXPORTS_H_
#define _KONSOLEKALENDAREXPORTS_H_

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


    bool exportAsTxt( QTextStream *ts, Event *event, QDate date );
    bool exportAsTxtShort( QTextStream *ts, Event *event, QDate date,
                           bool sameday  );
    bool exportAsCSV( QTextStream *ts, Event *event, QDate date );


  private:
    KonsoleKalendarVariables *m_variables;
    bool m_firstEntry;
    QString processField( QString field, QString dquote );

  };

}
#endif

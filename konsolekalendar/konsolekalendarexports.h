/*******************************************************************************
 * konsolekalendarexports.h                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
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

/**
 * @file konsolekalendarexports.h
 * Provides the KonsoleKalendarExports class definition.
 */

namespace KCal
{
  /**
   * Class to manage the Export functionality.
   * @author Tuukka Pasanen
   * @author Allen Winter
   */
  class KonsoleKalendarExports
  {
  public:

    /**
     * Constructs a KonsoleKalendarChange object from command line arguments.
     * @param vars is a KonsoleKalendarVariable object with Event information.
     */
    KonsoleKalendarExports( KonsoleKalendarVariables *vars = 0 );
    /**
     * Destructor
     */
    ~KonsoleKalendarExports();

    /**
     * Export the Event in Text Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     */
    bool exportAsTxt( QTextStream *ts, Event *event, QDate date );
    /**
     * Export the Event in Short Text Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     * @param sameday flags that this Event is on the same date as the
     * previously exported Event.
     */
    bool exportAsTxtShort( QTextStream *ts, Event *event, QDate date,
                           bool sameday );
    /**
     * Export the Event in Comma-Separated Values (CSV) Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     */
    bool exportAsCSV( QTextStream *ts, Event *event, QDate date );

  private:
    KonsoleKalendarVariables *m_variables;
    bool m_firstEntry;
    /**
     * Processes a field for Comma-Separated Value (CSV) compliance:
     *   1. Replaces double quotes by a pair of consecutive double quotes
     *   2. Surrounds field with double quotes
     * @param field is the field value to be processed.
     * @param dquote is a QString containing the double quote character.
     */
    QString processField( QString field, QString dquote );

  };

}
#endif

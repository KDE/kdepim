/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <klocale.h>

#include "hexerror.h"

static QString message;


const QString &hexError( int index )
{
  static QString messages[ Err_MAXERROR - Err_NoData ] = 
  {
    i18n("No data"),                                    // Err_NoData
    i18n("Insufficient memory"),                        // Err_NoMemory
    i18n("List is full"),                               // Err_ListFull
    i18n("Read operation failed"),                      // Err_ReadFailed
    i18n("Write operation failed"),                     // Err_WriteFailed
    i18n("Empty argument"),                             // Err_EmptyArgument
    i18n("Illegal argument"),                           // Err_IllegalArgument
    i18n("Null pointer argument"),                      // Err_NullArgument
    i18n("Wrap buffer"),                                // Err_WrapBuffer
    i18n("No match"),                                   // Err_NoMatch
    i18n("No data is selected"),                        // Err_NoSelection
    i18n("Empty document"),                             // Err_EmptyDocument
    i18n("No active document"),                         // Err_NoActiveDocument
    i18n("No data is marked"),                          // Err_NoMark
    i18n("Document is write protected"),                // Err_WriteProtect
    i18n("Document is resize protected"),               // Err_NoResize
    i18n("Operation was stopped"),                      // Err_Stop
    i18n("Illegal mode"),                               // Err_IllegalMode
    i18n("Program is busy, try again later"),           // Err_Busy
    i18n("Value is not within valid range"),            // Err_IllegalRange
    i18n("Operation was aborted"),                      // Err_OperationAborted
    i18n("KIO job in progress"),                        // Err_KioInProgress
    i18n("File could not be opened for writing"),       // Err_OpenWriteFailed
    i18n("File could not be opened for reading"),       // Err_OpenReadFailed
  }; 


  if( index < Err_NoData || index >= Err_MAXERROR )
  {
    message = i18n("Unknown error");
  }
  else
  {
    message = messages[ index - Err_NoData ];
  }

  return( message );
}


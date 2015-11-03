/***************************************************************************
                          filter_lnotes.h  -  Lotus Notes Structured Text mail import
                             -------------------
    begin                : Wed Feb 16, 2005
    copyright            : (C) 2005 by Robert Rockers
    email                : kconfigure@rockerssoft.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_LNOTES_HXX
#define MAILIMPORTER_FILTER_LNOTES_HXX

#include "filters.h"
/**imports Lotus Notes Structured Text Archives and archvies messages into KMail
 *@author Robert Rockers
 */
namespace MailImporter
{
class FilterLNotesPrivate;
class MAILIMPORTER_EXPORT FilterLNotes : public Filter
{
public:
    explicit FilterLNotes();
    ~FilterLNotes();
    /** Standard import filter... starting line for our import */
    void import() Q_DECL_OVERRIDE;

private:
    FilterLNotesPrivate *const d;
    /**
    * This is were all the real action is gonna be handled.
    * Gets called once for EACH file imported
    */
    void ImportLNotes(const QString &file);

};
}

#endif

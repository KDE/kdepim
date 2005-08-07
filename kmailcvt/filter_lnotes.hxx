/***************************************************************************
                          filter_lnotes.hxx  -  Lotus Notes Structured Text mail import
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

#ifndef FILTER_LNOTES_HXX
#define FILTER_LNOTES_HXX

#include "filters.hxx"

/**imports Lotus Notes Structured Text Archives and archvies messages into KMail
 *@author Robert Rockers
 */

class FilterLNotes : public Filter {

public:
    FilterLNotes();
    ~FilterLNotes();
    /** Standard import filter... starting line for our import */
    void import(FilterInfo *info);

private:
    /** the working directory */
    QDir dir;
    /** Our Filterinfo stuff... important methods for getting the email imported */
    FilterInfo * inf;
    /** which file (of totalFiles) is now in the work? */
    int currentFile;
    /** total number of files that get imported */
    int totalFiles;

    /** 
     * This is were all the real action is gonna be handled.  
     * Gets called once for EACH file imported 
     */
    void ImportLNotes(const QString& file);
    
};

#endif

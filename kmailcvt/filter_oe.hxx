/***************************************************************************
                          filter_oe.hxx  -  Outlook Express mail import
                             -------------------
    begin                : Sat Feb 1 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_OE_HXX
#define FILTER_OE_HXX

#include "filters.hxx"

/**imports outlook text messages into KMail
 *@author Laurence Anderson
 */

class FilterOE : public Filter {
  public:
    FilterOE();
    ~FilterOE();

    void import(FilterInfo *info);

  protected:
    void importMailBox( FilterInfo *info, const QString& fileName);
    void mbxImport( FilterInfo *info, QDataStream& ds);
    void dbxImport( FilterInfo *info, QDataStream& ds);
    void dbxReadIndex( FilterInfo *info, QDataStream& ds, int filePos);
    void dbxReadDataBlock( FilterInfo *info, QDataStream& ds, int filePos);
    void dbxReadEmail( FilterInfo *info, QDataStream& ds, int filePos);

  private: // Private methods
    FilterInfo * inf;
    /** which file (of totalFiles) is now in the work? */
    int currentFile;
    /** total number of files that get imported */
    int totalFiles;
    /** total emails in current file */
    int totalEmails;
    /** which email (of totalFiles) is now in the work? */
    int currentEmail;
    /** number of imported mails with flag 0x04 */
    int count0x04;
    /** number of imported mails with flag 0x84 */
    int count0x84;

    QString folderName;
};

#endif

// vim: ts=2 sw=2 et

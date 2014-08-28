/***************************************************************************
                          filter_oe.h  -  Outlook Express mail import
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

#ifndef MAILIMPORTER_FILTER_OE_HXX
#define MAILIMPORTER_FILTER_OE_HXX

#include "filters.h"

#include <QList>

/**
 *imports outlook text messages into KMail
 *@author Laurence Anderson
 */
#include "mailimporter_utils.h"
namespace MailImporter
{
class MAILIMPORTER_EXPORT FilterOE : public Filter
{
public:
    explicit FilterOE();
    ~FilterOE();

    void import();
    void importMails(const QString &maildir);

protected:
    void importMailBox(const QString &fileName);
    void mbxImport(QDataStream &ds);
    void dbxImport(QDataStream &ds);
    void dbxReadIndex(QDataStream &ds, int filePos);
    void dbxReadDataBlock(QDataStream &ds, int filePos);
    void dbxReadEmail(QDataStream &ds, int filePos);

    /** helperfunctions for folder structure support */
    QString parseFolderString(QDataStream &ds, int filePos);
    QString getFolderName(const QString &filename);

private: // Private methods
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

    /** true if the folderstructure is parsed */
    bool parsedFolder;
    /** true if the current parsing file is the folder file */
    bool currentIsFolderFile;

    /** Folder structure with following  4 entries:
      1. descriptive folder name
      2. filename
      3. ID of current folder
      4. ID of parent folder
    */
    typedef FolderStructureBase<4> FolderStructure;
    /** matrix with information about the folder structure*/
    QList<FolderStructure> folderStructure;
    typedef QList<FolderStructure>::Iterator FolderStructureIterator;

    /** name of the current folder */
    QString folderName;
    /** name of the chosen folder */

};
}

#endif

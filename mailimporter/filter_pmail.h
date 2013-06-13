/***************************************************************************
                          FilterPMail.h  -  Pegasus-Mail import
                             -------------------
    begin                : Sat Jan 6 2001
    copyright            : (C) 2001 by Holger Schurig
    email                : holgerschurig@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_PMAIL_HXX
#define MAILIMPORTER_FILTER_PMAIL_HXX

#include <QDir>

#include "filters.h"
#include "mailimporter_utils.h"
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterPMail : public Filter
{
public:
    explicit FilterPMail();
    ~FilterPMail();

    void import();
    void importMails( const QString & chosenDir );

protected:
    /** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
    void processFiles(const QString& mask,  void(FilterPMail::* workFunc)(const QString&) );
    /** this function imports one *.CNM message */
    void importNewMessage(const QString& file);
    /** this function imports one mail folder file (*.PMM) */
    void importMailFolder(const QString& file);
    /** imports a 'unix' format mail folder (*.MBX) */
    void importUnixMailFolder(const QString& file);
    /** this function recreate the folder structure */
    bool parseFolderMatrix(const QString& chosenDir);
    /** this function parse the folder structure */
    QString getFolderName(const QString& ID);
    
private:
    /** the working directory */
    QDir dir;

    /** Folder structure here has 5 entries. */
    typedef FolderStructureBase<5> FolderStructure;
    /** List with the folder matrix, which contains following strings:
      1. type (2 for root-folder, 1 for folder, 0 for mailarchiv)
      2. type (1 for root-folder, 3 for folder, 0 for mailarchiv)
      3. "ID:flag:filename" of folder/archiv
      4. "ID:name" of parent folder
      5. name of folder/archiv
  */
    QList<FolderStructure> folderMatrix;
    typedef QList<FolderStructure>::Iterator FolderStructureIterator;

    bool folderParsed;
    
    /** which file (of totalFiles) is now in the work? */
    int currentFile;
    /** total number of files that get imported */
    int totalFiles;
    
};
}
#endif

/***************************************************************************
                          FilterPMail.hxx  -  Pegasus-Mail import
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

#ifndef FILTER_PMAIL_HXX
#define FILTER_PMAIL_HXX

#include <tqdir.h>
#include <tqvaluelist.h>

#include "filters.hxx"

class FilterPMail : public Filter
{
public:
    FilterPMail();
    ~FilterPMail();

    void import(FilterInfo *info);

protected:
    /** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
    void processFiles(const TQString& mask,  void(FilterPMail::* workFunc)(const TQString&) );
    /** this function imports one *.CNM message */
    void importNewMessage(const TQString& file);
    /** this function imports one mail folder file (*.PMM) */
    void importMailFolder(const TQString& file);
    /** imports a 'unix' format mail folder (*.MBX) */
    void importUnixMailFolder(const TQString& file);
    /** this function recreate the folder structure */
    bool parseFolderMatrix();
    /** this function parse the folder structure */
    TQString getFolderName(TQString ID); 
    
private:
    /** the working directory */
    TQDir dir;
    /**  pointer to the info */
    FilterInfo * inf;

    /** Folder structure here has 5 entries. */
    typedef FolderStructureBase<5> FolderStructure;
    /** List with the folder matrix, which contains following strings:
	1. type (2 for root-folder, 1 for folder, 0 for mailarchiv)
	2. type (1 for root-folder, 3 for folder, 0 for mailarchiv)  
	3. "ID:flag:filename" of folder/archiv   
	4. "ID:name" of parent folder
	5. name of folder/archiv
    */
    TQValueList<FolderStructure> folderMatrix;
    typedef TQValueList<FolderStructure>::Iterator FolderStructureIterator;
    
    bool folderParsed;
    
    TQString chosenDir;
    
    /** which file (of totalFiles) is now in the work? */
    int currentFile;
    /** total number of files that get imported */
    int totalFiles;
    
};
#endif

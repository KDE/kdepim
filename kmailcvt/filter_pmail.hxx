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

#include "filters.hxx"

/**imports Pegasus-Mail 3.xx messages into KMail
  *@author root
  */

class FilterPMail : public Filter, public FilterFactory< FilterPMail >  {
public:
	FilterPMail();
	~FilterPMail();
	
  void import(FilterInfo *info);	

protected:
  /** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
  void processFiles(QString mask,  void(FilterPMail::* workFunc)(QString) );
  /** this function imports one *.CNM message */
  void importNewMessage(QString file);
  /** this function imports one mail folder file (*.PMM) */
  void importMailFolder(QString file);
  /** imports a 'unix' format mail folder (*.MBX) */
  void importUnixMailFolder(QString file);
private:
  /** the working directory */
  QDir dir;
  /**  */
  FilterInfo * inf;
  /** which file (of totalFiles) is now in the work? */
  int currentFile;
  /** total number of files that get imported */
  int totalFiles;
};

#endif

/***************************************************************************
                          filter_plain.hxx  -  Plain mail import
                             -------------------
    begin                : Fri Jun 24 2002
    copyright            : (C) 2002 by Laurence Anderson
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

#ifndef FILTER_PLAIN_HXX
#define FILTER_PLAIN_HXX

#include "filters.hxx"

/**imports Plain text messages into KMail
  *@author laurence
  */

class filter_plain : public filter  {
private:
  /** the working directory */
  QString mailDir;
  /**  */
  filterInfo * inf;
  /** which file (of totalFiles) is now in the work? */
  int currentFile;
  /** total number of files that get imported */
  int totalFiles;
public:
	filter_plain();
	~filter_plain();
public:
  void import(filterInfo *info);
private: // Private methods
  /** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
  void processFiles(QString filter);
  /** counts all files with mask (e.g. '*.cnm') in in a directory */
  int countFiles(QString filter);
};

#endif

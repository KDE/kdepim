/***************************************************************************
            filter_kmail_maildir.hxx  -  Kmail maildir mail import
                             -------------------
    begin                : April 06 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FILTER_KMAIL_MAILDIR_HXX
#define FILTER_KMAIL_MAILDIR_HXX

#include "filters.hxx"

/**
 * Imports KMail mail folder with maildir format recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
class FilterKMail_maildir : public Filter
{
public:
  FilterKMail_maildir(void);
  ~FilterKMail_maildir(void);

  void import(FilterInfo *info);

private:
  QString mailDir;

  void importDirContents(FilterInfo*, const QString&, const QString&);
};

#endif

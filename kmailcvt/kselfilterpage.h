/***************************************************************************
                          kselfilterpage.h  -  description
                             -------------------
    begin                : Fri Jan 17 2003
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

#ifndef KSELFILTERPAGE_H
#define KSELFILTERPAGE_H

#include "kselfilterpagedlg.h"
#include "harray.hxx"
#include "filters.hxx"
#include <kaboutdata.h>

class KSelFilterPage : public KSelFilterPageDlg  {
	Q_OBJECT
public:
	KSelFilterPage(QWidget *parent=0, const char *name=0);
	~KSelFilterPage();
  public:
    void  addFilter(filter *f);
    filter *getSelectedFilter(void);
    void  setAuthors(KAboutData& data);
   private:
     harray<filter *> F;
     QWidget *_parent;
     filterInfo *info;
};

#endif

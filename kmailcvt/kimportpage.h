/***************************************************************************
                          kimportpage.h  -  description
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

#ifndef KIMPORTPAGE_H
#define KIMPORTPAGE_H

#include "kimportpagedlg.h"

class KImportPage : public KImportPageDlg  {
	Q_OBJECT
public:
	KImportPage(QWidget *parent=0, const char *name=0);
	~KImportPage();
};

#endif

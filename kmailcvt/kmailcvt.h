/***************************************************************************
                          kmailcvt2.h  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMAILCVT2_H
#define KMAILCVT2_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>

#include <klocale.h>

#include <stdio.h>


#define KMAILCVT_VERSION " v2.1"
#define KMAILCVT         I18N_NOOP("KMail & KAddressBook Import Filters")

#include <kapplication.h>
#include <kwizard.h>

#include "kimportpage.h"
#include "kselfilterpage.h"
  
/** KMailCVT is the base class of the project */
class Kmailcvt2 : public KWizard {
	Q_OBJECT
public:
	Kmailcvt2(QWidget* parent=0, const char *name=0);
	~Kmailcvt2();

	virtual void next();
	virtual void back();
public slots:
	void help();
private:
	KSelFilterPage* selfilterpage;
	KImportPage* importpage;
	QWidget *_parent;
};

#include "filters.hxx"


#endif

/***************************************************************************
                          kmailcvt.h  -  description
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

#define KMAILCVT_VERSION "3"
#define KMAILCVT         I18N_NOOP("KMail Import Filters")

#include <kapplication.h>
#include <kwizard.h>

#include "filters.hxx"
#include "kimportpage.h"
#include "kselfilterpage.h"
  
/** KMailCVT is the base class of the project */
class KMailCVT : public KWizard {
	Q_OBJECT
public:
	KMailCVT(QWidget* parent=0, const char *name=0);
	~KMailCVT();

	virtual void next();
	virtual void reject();
public slots:
	void help();
private:
	KSelFilterPage* selfilterpage;
	KImportPage* importpage;
	Filter::List filters;
	QWidget *_parent;
};

#endif

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

#ifndef KMAILCVT_H
#define KMAILCVT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kwizard.h>

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
};

#endif

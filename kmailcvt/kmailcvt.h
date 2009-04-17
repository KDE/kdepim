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

#include <kapplication.h>
#include <KAssistantDialog>

class KPageWidgetItem;
class KSelFilterPage;
class KImportPage;


/** KMailCVT is the base class of the project */
class KMailCVT : public KAssistantDialog {
	Q_OBJECT
public:
	KMailCVT(QWidget* parent=0);
	~KMailCVT();

	virtual void next();
	virtual void reject();
public slots:
	void help();
private:
	KPageWidgetItem* page1;
	KPageWidgetItem* page2;
        void endImport();
	KSelFilterPage *selfilterpage;
	KImportPage *importpage;

};

#endif

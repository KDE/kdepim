/***************************************************************************
                          knfilterconfigwidget.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kconfig.h>
#include <klocale.h>

#include "knfilterconfigwidget.h"
#include "utilities.h"
#include <qlayout.h>

KNFilterConfigWidget::KNFilterConfigWidget(QWidget *parent, const char *name ) :
	QTabWidget(parent,name)
{
	QWidget *sf, *add;
	sf=new QWidget(this);
	subject=new KNStringFilterWidget(i18n("Subject"), sf);
	from=new KNStringFilterWidget(i18n("From"), sf);
	QVBoxLayout *sfL=new QVBoxLayout(sf, 10);
	sfL->addWidget(subject);
	sfL->addWidget(from);
	sfL->addStretch(1);
	sfL->activate();	
	addTab(sf, i18n("Subject + From"));
			
	status=new KNStatusFilterWidget(this);
	addTab(status, i18n("Status"));
		
	add=new QWidget(this);
	age=new KNRangeFilterWidget(i18n("Age"), 0, 999, add);
	lines=new KNRangeFilterWidget(i18n("Lines"), 0, 999, add);
	score=new KNRangeFilterWidget(i18n("Score"), 0, 100, add);
	QVBoxLayout *addL=new QVBoxLayout(add, 10);
	addL->addWidget(age);
	addL->addWidget(lines);
	addL->addWidget(score);
	addL->addStretch(1);
	addL->activate();
	addTab(add, i18n("Additional"));
}	



KNFilterConfigWidget::~KNFilterConfigWidget()
{
}



void KNFilterConfigWidget::reset()
{
	from->clear();
	subject->clear();
	age->clear();
	lines->clear();
	score->clear();
	status->clear();
}

// -----------------------------------------------------------------------------

#include "knfilterconfigwidget.moc"

/***************************************************************************
                          kimportpage.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by Laurence Anderso
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

#include "kimportpage.h"

#include <kapplication.h>
#include <kstandarddirs.h>

KImportPage::KImportPage(QWidget *parent )
    : QWidget(parent)
{
    mWidget = new Ui::KImportPageDlg;
    mWidget->setupUi( this );
    mWidget->mIntroSidebar->setPixmap(KStandardDirs::locate("data", QLatin1String("kmailcvt/pics/step1.png")));
}

KImportPage::~KImportPage()
{
    delete mWidget;
}

Ui::KImportPageDlg *KImportPage::widget()
{
    return mWidget;
}



#include "moc_kimportpage.cpp"

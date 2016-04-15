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

#include "manualimportmailpage.h"

#include <QStandardPaths>

ManualImportMailPage::ManualImportMailPage(QWidget *parent)
    : QWidget(parent)
{
    mWidget = new Ui::ManualImportMailPage;
    mWidget->setupUi(this);
    mWidget->mIntroSidebar->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("importwizard/pics/step1.png")));
}

ManualImportMailPage::~ManualImportMailPage()
{
    delete mWidget;
}

Ui::ManualImportMailPage *ManualImportMailPage::widget() const
{
    return mWidget;
}


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

#ifndef ManualImportMailPage_H
#define ManualImportMailPage_H

#include "ui_manualimportmailpage.h"

class ManualImportMailPage : public QWidget
{
    Q_OBJECT
public:
    explicit ManualImportMailPage(QWidget *parent = Q_NULLPTR);
    ~ManualImportMailPage();

    Ui::ManualImportMailPage *widget() const;
private:
    Ui::ManualImportMailPage *mWidget;
};

#endif

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

#include "ui_manualselectfilterpage.h"
#include <QList>
namespace MailImporter
{
class Filter;
}

class ManualSelectFilterPage : public QWidget
{
    Q_OBJECT
public:
    explicit ManualSelectFilterPage(QWidget *parent = Q_NULLPTR);
    ~ManualSelectFilterPage();

    void  addFilter(MailImporter::Filter *f);
    MailImporter::Filter *getSelectedFilter() const;
    bool removeDupMsg_checked() const;

    Ui::ManualSelectFilterPage *widget() const;

private Q_SLOTS:
    void filterSelected(int i);

private:
    Ui::ManualSelectFilterPage *mWidget;
    QList<MailImporter::Filter *> mFilterList;

};

#endif

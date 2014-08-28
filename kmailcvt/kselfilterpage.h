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

#include "ui_kselfilterpagedlg.h"
#include <QList>
namespace MailImporter
{
class Filter;
}

class KSelFilterPage : public QWidget
{
    Q_OBJECT
public:
    explicit KSelFilterPage(QWidget *parent = 0);
    ~KSelFilterPage();

    void  addFilter(MailImporter::Filter *f);
    MailImporter::Filter *getSelectedFilter();
    bool removeDupMsg_checked() const;

    Ui::KSelFilterPageDlg *widget();

private Q_SLOTS:
    void filterSelected(int i);

private:
    Ui::KSelFilterPageDlg *mWidget;
    QList<MailImporter::Filter *> mFilterList;

};

#endif
